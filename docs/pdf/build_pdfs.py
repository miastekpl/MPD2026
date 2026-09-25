#!/usr/bin/env python3
"""Buduje PDF-y z dokumentacji MPD2026 (Markdown -> HTML -> PDF przez Chrome/Edge bez interfejsu).

Wymagania: pip install markdown pypdf reportlab ; zainstalowany Google Chrome lub Microsoft Edge.
Uruchomienie:  python docs/pdf/build_pdfs.py
Wyniki w docs/pdf/: pojedyncze PDF-y, schematy/*.pdf oraz MPD2026_Dokumentacja_komplet.pdf
"""
import io
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import markdown
from pypdf import PdfReader, PdfWriter
from reportlab.lib.pagesizes import A4
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfgen import canvas

ROOT = Path(__file__).resolve().parents[2]
DOCS = ROOT / "docs"
OUT = DOCS / "pdf"
TMP = OUT / "_tmp"

# (plik źródłowy, nazwa PDF, tytuł)
DOCUMENTS = [
    (ROOT / "README.md", "00_README.pdf", "README - przegląd projektu"),
    (DOCS / "INSTRUKCJA_OBSLUGI.md", "01_Instrukcja_obslugi.pdf", "Instrukcja obsługi"),
    (DOCS / "INSTRUKCJA_TERENOWA.md", "02_Instrukcja_terenowa.pdf", "Instrukcja obsługi w terenie"),
    (DOCS / "SCHEMAT_PODLACZEN.md", "03_Schemat_polaczen.pdf", "Schemat połączeń i dokumentacja sprzętowa"),
    (DOCS / "WIZUALIZACJE.md", "04_Wizualizacje_panelu.pdf", "Wizualizacje panelu"),
    (DOCS / "MODUL_WYSWIETLACZA.md", "05_Modul_wyswietlacza.pdf", "Moduł wyświetlacza 7\""),
    (DOCS / "LACZE_PRZEWODOWE.md", "06_Lacze_przewodowe.pdf", "Łącze przewodowe RS-485"),
    (DOCS / "API_WWW.md", "07_API_sterownika.pdf", "API sterownika"),
    (DOCS / "ROADMAP.md", "08_Roadmap.pdf", "Roadmap (wersja bazowa)"),
    (DOCS / "CODE_REVIEW.md", "09_Przeglad_kodu.pdf", "Przegląd kodu (wersja bazowa)"),
    (ROOT / "CHANGELOG.md", "10_Changelog.pdf", "Historia zmian"),
]

SVGS = [
    ("schemat_polaczen.svg", "Schemat polaczen elektronicznych"),
    ("schemat_zlacza_wiazka.svg", "Wiazka okablowania - zlacza J1-J6"),
    ("schemat_lacze_rs485.svg", "Lacze przewodowe RS-485"),
    ("wzorce_w_skali.svg", "Rysunki wzorcow w skali"),
    ("ekran_roboczy.svg", "Makieta ekranu roboczego"),
    ("panel_A_kabinowy.svg", "Propozycja A - kabinowy"),
    ("panel_B_pas_pod_ekranem.svg", "Propozycja B - pas pod ekranem"),
    ("panel_C_pionowy.svg", "Propozycja C - pionowy"),
    ("panel_D_estop.svg", "Propozycja D - grzybek STOP"),
]

CSS = """
@page { size: A4; margin: 16mm 14mm 18mm 14mm; }
* { box-sizing: border-box; }
body { font-family: 'Segoe UI', Arial, sans-serif; font-size: 10pt; line-height: 1.45; color: #1a1a1a; }
h1 { font-size: 20pt; color: #0b1f4a; border-bottom: 3px solid #0b1f4a; padding-bottom: 4px; margin-top: 0; }
h2 { font-size: 14.5pt; color: #0b1f4a; border-bottom: 1px solid #b8c4dc; padding-bottom: 2px; margin-top: 20px; page-break-after: avoid; }
h3 { font-size: 12pt; color: #16346e; margin-top: 14px; page-break-after: avoid; }
h4 { font-size: 10.5pt; color: #16346e; page-break-after: avoid; }
p, li { orphans: 3; widows: 3; }
a { color: #1f52d6; text-decoration: none; }
table { border-collapse: collapse; width: 100%; margin: 8px 0 12px 0; font-size: 8.6pt; page-break-inside: auto; }
tr { page-break-inside: avoid; }
th { background: #e4ebf7; color: #0b1f4a; text-align: left; }
th, td { border: 1px solid #b9c3d6; padding: 3px 5px; vertical-align: top; }
code { font-family: Consolas, 'Courier New', monospace; font-size: 8.8pt; background: #f1f3f8; padding: 0 2px; border-radius: 2px; }
pre { background: #f5f7fb; border: 1px solid #d3d9e6; border-radius: 4px; padding: 6px 8px; font-family: Consolas, 'Courier New', monospace;
      font-size: 7pt; line-height: 1.25; white-space: pre; overflow: hidden; page-break-inside: avoid; }
pre code { background: none; padding: 0; font-size: inherit; }
blockquote { border-left: 4px solid #f0b400; background: #fff8e1; margin: 8px 0; padding: 4px 10px; }
img { max-width: 100%; height: auto; display: block; margin: 8px auto; page-break-inside: avoid; }
hr { border: 0; border-top: 1px solid #c5cddd; margin: 14px 0; }
pre.mermaid { background: #ffffff; border: 1px solid #d3d9e6; text-align: center; white-space: normal; font-size: 9pt; }
.mermaid svg { max-width: 100%; height: auto; }
"""

MERMAID_SCRIPT = """
<script src="https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js"></script>
<script>mermaid.initialize({startOnLoad:true, securityLevel:'loose', theme:'default', flowchart:{htmlLabels:true}});</script>
"""

FONT_PATH = Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts" / "arial.ttf"
FONT_BOLD = Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts" / "arialbd.ttf"


def find_browser():
    candidates = [
        r"C:\Program Files\Google\Chrome\Application\chrome.exe",
        r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
    ]
    for c in candidates:
        if os.path.exists(c):
            return c
    found = shutil.which("chrome") or shutil.which("msedge") or shutil.which("google-chrome")
    if found:
        return found
    sys.exit("Nie znaleziono Chrome/Edge.")


BROWSER = find_browser()
PDF_NAMES = {src.name: pdf for src, pdf, _ in DOCUMENTS}


def rewrite_links(html):
    """Linki do plików .md zamień na odpowiadające PDF-y; usuń kotwice do nieistniejących plików."""
    def repl(m):
        href = m.group(1)
        base = href.split("#")[0]
        name = os.path.basename(base)
        if name in PDF_NAMES:
            return 'href="' + PDF_NAMES[name] + '"'
        return m.group(0)
    return re.sub(r'href="([^"]+\.md(?:#[^"]*)?)"', repl, html)


def md_to_html(src, title):
    text = src.read_text(encoding="utf-8")
    md = markdown.Markdown(extensions=["tables", "fenced_code", "toc", "sane_lists", "attr_list"],
                           extension_configs={"toc": {"permalink": False}})
    body = md.convert(text)
    # bloki mermaid -> <pre class="mermaid">
    body = re.sub(r'<pre><code class="language-mermaid">(.*?)</code></pre>',
                  lambda m: '<pre class="mermaid">' + m.group(1) + '</pre>', body, flags=re.S)
    body = rewrite_links(body)
    has_mermaid = "class=\"mermaid\"" in body
    base = src.parent.as_uri() + "/"
    html = (f'<!doctype html><html lang="pl"><head><meta charset="utf-8"><title>{title}</title>'
            f'<base href="{base}"><style>{CSS}</style></head><body>{body}'
            f'{MERMAID_SCRIPT if has_mermaid else ""}</body></html>')
    return html, has_mermaid


def print_pdf(html_path, pdf_path, mermaid=False):
    budget = "40000" if mermaid else "8000"
    cmd = [BROWSER, "--headless=new", "--disable-gpu", "--no-pdf-header-footer",
           f"--virtual-time-budget={budget}", "--run-all-compositor-stages-before-draw",
           f"--print-to-pdf={pdf_path}", html_path.as_uri()]
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=180)


def svg_to_pdf(svg_path, pdf_path):
    text = svg_path.read_text(encoding="utf-8")
    m = re.search(r'viewBox="0 0 (\d+) (\d+)"', text)
    w, h = (int(m.group(1)), int(m.group(2))) if m else (1600, 1000)
    html = (f'<!doctype html><html><head><meta charset="utf-8"><style>@page {{ size: {w}px {h}px; margin: 0; }}'
            f'html,body{{margin:0;padding:0;background:#fff}}img{{display:block;width:{w}px;height:{h}px}}</style></head>'
            f'<body><img src="{svg_path.as_uri()}"></body></html>')
    hp = TMP / (svg_path.stem + ".html")
    hp.write_text(html, encoding="utf-8")
    print_pdf(hp, pdf_path)


def add_footer(pdf_in, pdf_out, label):
    """Numery stron i etykieta w stopce (polskie znaki: czcionka Arial)."""
    if "ArialPL" not in pdfmetrics.getRegisteredFontNames():
        pdfmetrics.registerFont(TTFont("ArialPL", str(FONT_PATH)))
    reader = PdfReader(str(pdf_in))
    writer = PdfWriter()
    total = len(reader.pages)
    for i, page in enumerate(reader.pages, start=1):
        w = float(page.mediabox.width)
        h = float(page.mediabox.height)
        buf = io.BytesIO()
        c = canvas.Canvas(buf, pagesize=(w, h))
        c.setFont("ArialPL", 8)
        c.setFillGray(0.4)
        c.drawString(40, 18, label)
        c.drawRightString(w - 40, 18, f"strona {i} / {total}")
        c.save()
        buf.seek(0)
        page.merge_page(PdfReader(buf).pages[0])
        writer.add_page(page)
    with open(pdf_out, "wb") as f:
        writer.write(f)


def title_page(pdf_path, entries):
    pdfmetrics.registerFont(TTFont("ArialPL", str(FONT_PATH)))
    pdfmetrics.registerFont(TTFont("ArialPL-B", str(FONT_BOLD)))
    c = canvas.Canvas(str(pdf_path), pagesize=A4)
    w, h = A4
    c.setFillColorRGB(0.043, 0.12, 0.29)
    c.rect(0, h - 220, w, 220, fill=1, stroke=0)
    c.setFillColorRGB(1, 1, 1)
    c.setFont("ArialPL-B", 30)
    c.drawString(50, h - 110, "MPD2026")
    c.setFont("ArialPL", 16)
    c.drawString(50, h - 145, "Komputer pokładowy malowarki pasów drogowych")
    c.drawString(50, h - 170, "Kompletna dokumentacja")
    c.setFillColorRGB(0.1, 0.1, 0.1)
    c.setFont("ArialPL-B", 13)
    c.drawString(50, h - 270, "Spis dokumentów")
    c.setFont("ArialPL", 11)
    y = h - 300
    for title, start_page in entries:
        c.drawString(60, y, title)
        c.drawRightString(w - 60, y, f"str. {start_page}")
        y -= 20
    c.setFont("ArialPL", 9)
    c.setFillGray(0.4)
    c.drawString(50, 60, "Sterownik Trassar (ESP32-S3) + moduł wyświetlacza 7\" — repozytorium miastekpl/MPD2026")
    c.drawString(50, 46, "Wygenerowano skryptem docs/pdf/build_pdfs.py")
    c.save()


def main():
    OUT.mkdir(exist_ok=True)
    (OUT / "schematy").mkdir(exist_ok=True)
    TMP.mkdir(exist_ok=True)

    built = []
    for src, pdf_name, title in DOCUMENTS:
        if not src.exists():
            print("brak", src)
            continue
        html, has_mermaid = md_to_html(src, title)
        html_path = TMP / (src.stem + ".html")
        html_path.write_text(html, encoding="utf-8")
        raw = TMP / (src.stem + "_raw.pdf")
        print("PDF:", pdf_name, "(mermaid)" if has_mermaid else "")
        print_pdf(html_path, raw, mermaid=has_mermaid)
        final = OUT / pdf_name
        add_footer(raw, final, f"MPD2026 — {title}")
        built.append((title, final))

    for name, title in SVGS:
        svg = DOCS / "schematy" / name
        if not svg.exists():
            continue
        out = OUT / "schematy" / (svg.stem + ".pdf")
        print("SVG:", out.name)
        svg_to_pdf(svg, out)

    # komplet z zakładkami
    entries = []
    page = 2
    merged = PdfWriter()
    counts = []
    for title, pdf in built:
        n = len(PdfReader(str(pdf)).pages)
        counts.append((title, pdf, n))
    for title, pdf, n in counts:
        entries.append((title, page))
        page += n
    cover = TMP / "cover.pdf"
    title_page(cover, entries)
    merged.append(str(cover))
    for title, pdf, n in counts:
        start = len(merged.pages)
        merged.append(str(pdf))
        merged.add_outline_item(title, start)
    merged.add_metadata({"/Title": "MPD2026 - kompletna dokumentacja", "/Author": "miastekpl/MPD2026"})
    complete = OUT / "MPD2026_Dokumentacja_komplet.pdf"
    with open(complete, "wb") as f:
        merged.write(f)
    print("KOMPLET:", complete.name, len(merged.pages), "stron")

    shutil.rmtree(TMP, ignore_errors=True)


if __name__ == "__main__":
    main()

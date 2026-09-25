# MPD2026 — Instrukcja obsługi w terenie

Praktyczny przewodnik dla ekipy pracującej na drodze: od przygotowania dnia pracy, przez malowanie poszczególnych
typów linii, po awarie i zakończenie zmiany. Uzupełnia [INSTRUKCJĘ OBSŁUGI](INSTRUKCJA_OBSLUGI.md) (opis funkcji
urządzenia); tutaj są **procedury, kolejność działań, listy kontrolne i decyzje**.

> **Zakres i zastrzeżenia.** Opis działania komputera pochodzi z kodu i dokumentacji systemu. Zalecenia dotyczące farby,
> pogody i organizacji ruchu są **ogólnymi dobrymi praktykami** — wiążące są **karta techniczna użytej farby**, wymagania
> zleceniodawcy, **zatwierdzony projekt organizacji ruchu** i obowiązujące przepisy. Wartości orientacyjne oznaczono słowem
> „zwykle" lub „orientacyjnie".

## Spis treści

0. [Zasady złote](#0-zasady-złote)
1. [Zespół i BHP](#1-zespół-i-bhp)
2. [Wyposażenie i zaplecze](#2-wyposażenie-i-zaplecze)
3. [Przygotowanie dzień wcześniej](#3-przygotowanie-dzień-wcześniej)
4. [Uruchomienie rano — procedura krok po kroku](#4-uruchomienie-rano--procedura-krok-po-kroku)
5. [Rozpoznanie odcinka i plan prac](#5-rozpoznanie-odcinka-i-plan-prac)
6. [Pogoda i podłoże](#6-pogoda-i-podłoże)
7. [Parametry robocze: prędkość, farba, zasięg](#7-parametry-robocze-prędkość-farba-zasięg)
8. [Procedury malowania według typu linii](#8-procedury-malowania-według-typu-linii)
9. [Zmiana wzorca, łączenie odcinków, przejazdy](#9-zmiana-wzorca-łączenie-odcinków-przejazdy)
10. [Postoje, pauzy i praca w ruchu](#10-postoje-pauzy-i-praca-w-ruchu)
11. [Farba i tankowanie](#11-farba-i-tankowanie)
12. [Dokumentacja pracy](#12-dokumentacja-pracy)
13. [Sytuacje awaryjne](#13-sytuacje-awaryjne)
14. [Zakończenie zmiany](#14-zakończenie-zmiany)
15. [Konserwacja](#15-konserwacja)
16. [Usterki terenowe — tabela szybkiej diagnozy](#16-usterki-terenowe--tabela-szybkiej-diagnozy)
17. [Karty do wydruku](#17-karty-do-wydruku)
18. [Ściągawka](#18-ściągawka)

---

## 0. Zasady złote

1. **Fizyczny STOP jest zawsze pierwszym środkiem.** STOP na ekranie 7" działa przez łącze radiowe/kablowe i może nie dotrzeć;
   fizyczny STOP sterownika (panel, pilot, pedał) działa zawsze i wyłącza pistolety sprzętowo.
2. **Nie jedź z włączonym „MALOWANIE" po jezdni, której nie chcesz malować.** Przejazdy transportowe — **STOP** (nie pauza).
3. **Przed każdym dniem pracy: kalibracja sprawdzona, test wszystkich sześciu pistoletów, test STOP.**
4. **Nie ufaj ekranowi bardziej niż jezdni.** Po pierwszych 20–30 m każdego wzorca sprawdź na nawierzchni długość
   kreski/przerwy, szerokość i położenie.
5. **Poziom farby jest szacowany** z powierzchni. Po tankowaniu i przy zmianie farby sprawdź go z rzeczywistym stanem zbiornika.
6. **Jedna osoba obsługuje komputer, druga obserwuje jezdnię i ruch.** Operator nie patrzy na ekran, gdy maszyna jedzie
   szybciej niż krok.
7. **Każdy alarm ma przyczynę.** „ZA SZYBKO" i „ANOMALIA PISTOLETU" nie wyciszaj — usuń przyczynę.
8. **STOP kończy etap i zapisuje raport.** Nowy START zaczyna nowy etap i zeruje liczniki sesji — zanotuj wartości przed
   kolejnym STARTEM, jeśli są potrzebne.
9. **Nie wciskaj joysticka sterownika przy włączaniu zasilania.** (Pin rozruchowy; może uniemożliwić start.)
10. **Wątpliwość = zatrzymaj maszynę.**

---

## 1. Zespół i BHP

### 1.1 Role

| Rola | Zadania |
|------|---------|
| **Operator** | obsługuje komputer, decyduje o START/PAUZA/STOP, zmienia wzorce, obserwuje alarmy |
| **Pomocnik / kierowca** | prowadzi maszynę (jeśli inna osoba), obserwuje jezdnię, pilnuje geometrii i punktów startu |
| **Osoba zabezpieczająca ruch** | zabezpieczenie odcinka robót zgodnie z projektem organizacji ruchu |
| **Odpowiedzialny za zmianę** | dokumentacja, farba, rozliczenie, kontakt ze zleceniodawcą |

Przy małej ekipie funkcje można łączyć, ale **osoba prowadząca maszynę nie obsługuje menu w czasie jazdy roboczej**.

### 1.2 Środki ochrony i zasady

- Odzież ostrzegawcza, obuwie ochronne, rękawice odporne na farbę i rozpuszczalniki, okulary, ochrona dróg oddechowych
  przy natrysku wg karty charakterystyki farby.
- Praca na jezdni wyłącznie w wyznaczonym, zabezpieczonym odcinku.
- Pistolety i zawory pracują z farbą pod ciśnieniem: **przed czynnościami przy pistoletach zatrzymaj maszynę fizycznym STOP,
  zdejmij ciśnienie i odłącz zasilanie**.
- Nie dotykaj przekaźników ani złączy J2 przy załączonym zasilaniu zaworów (12/24 V).
- Gaśnica proszkowa/CO₂ w zasięgu; apteczka; środki do neutralizacji rozlewu.
- Farby rozpuszczalnikowe: brak źródeł zapłonu, wentylacja, uziemienie wg karty charakterystyki.

---

## 2. Wyposażenie i zaplecze

### 2.1 Lista rzeczy do zabrania

| Kategoria | Pozycja | Uwagi |
|-----------|---------|-------|
| Komputer | sterownik Trassar z modułem 7", pilot przewodowy (J4), pedał (J5) | sprawdzone, złącza czyste |
| Zasilanie | akumulator/instalacja 12/24 V, przetwornica 5 V (≥ 3 A), przewody zapasowe | |
| Karta SD | 1–2 zapasowe karty FAT32 (≥ 1 GB) | pierwsza w sterowniku, druga zapasowa |
| Kalibracja | **taśma miernicza ≥ 10 m** (najlepiej 20–30 m), kreda/farba do znaczenia | taśma stalowa lub szklana |
| Farba i rozpuszczalnik | farba wg projektu, rozpuszczalnik/płyn do przepłukania | zapas 10–15 % ponad plan |
| Osprzęt pistoletów | zapasowe dysze (każdej szerokości), uszczelki, filtry, węże | |
| Elektryka | bezpieczniki, tulejki, złącza, taśma izolacyjna, zapasowy przekaźnik/moduł 6-kanałowy, multimetr | |
| Narzędzia | klucze do dysz, szczotki, pędzelki, ściereczki, opaski, zestaw serwisowy | |
| Oznakowanie robót | stożki, znaki, latarnie ostrzegawcze | wg projektu organizacji ruchu |
| Dokumenty | projekt oznakowania, projekt organizacji ruchu, karta techniczna farby, dziennik prac (karta E) | |
| Pomiar | termometr do podłoża, higrometr (jeśli dostępny) | patrz rozdz. 6 |
| Pozostałe | kamera/telefon do zdjęć, zapasowy ładowarka/powerbank do telefonu | |

### 2.2 Części zamienne (minimum)

- Moduł przekaźników 6-kanałowy 5 V (lub pojedyncze przekaźniki).
- Dysze do każdego pistoletu (12 cm i 24 cm), uszczelki, sitka.
- Kabel enkodera i enkoder KY-040 (lub gotowa wiązka).
- Bezpieczniki PTC/TVS, przewody z tulejkami, złącza J1–J5.
- Zapasowa karta SD i zapasowa bateria CR2032 do RTC.

---

## 3. Przygotowanie dzień wcześniej

1. **Karta SD:** sprawdź wolne miejsce, w razie potrzeby zarchiwizuj i wyczyść stare raporty (najpierw kopia na komputer).
2. **Data i godzina:** sprawdź zegar (RTC). Błędna data psuje nazwy raportów i tras.
3. **Bateria/przetwornica:** sprawdź napięcie akumulatora i sprawność przetwornicy 5 V (pomiar 4,75–5,25 V).
4. **Farba:** zbiornik napełniony, zapas na plan prac; sprawdź datę ważności i temperaturę przechowywania.
5. **Pistolety:** demontaż, czyszczenie dysz, kontrola uszczelek; montaż i próba szczelności.
6. **Enkoder i koło pomiarowe:** kontrola stanu, dokręcenia, czystości bieżnika; wymiana koła = **nowa kalibracja**.
7. **Kable i złącza J1–J5:** oględziny, zabezpieczenie przed wilgocią; w razie potrzeby przeczyszczenie styków.
8. **Antena GPS:** czysta, w miejscu z widokiem na niebo.
9. **Plan prac:** wypisz odcinki, wzorce, długości i kolejność (rozdz. 5); policz zapotrzebowanie na farbę (rozdz. 7).
10. **Ładowanie:** telefony/tablety do zdjęć, powerbank.
11. **Test na sucho** (bez farby): włącz system, sprawdź połączenie modułu 7", wykonaj *Czyszczenie dysz* wszystkich pistoletów.

---

## 4. Uruchomienie rano — procedura krok po kroku

Wykonuj **na miejscu prac, na utwardzonym podłożu, poza ruchem**. Oczekiwany wynik każdego kroku jest w prawej kolumnie.

| # | Czynność | Oczekiwany wynik |
|---|----------|------------------|
| 1 | Oględziny maszyny: kable, złącza, wycieki, mocowanie pistoletów, koło pomiarowe | brak uszkodzeń, wycieków |
| 2 | Podłącz zasilanie 5 V (J1) i moduł 7"; **nie wciskaj joysticka** | włączają się oba ekrany |
| 3 | Sterownik: ekran startowy z QR (SSID, hasło, adres) | widoczne SSID `TrassarV3` i hasło 8 znaków |
| 4 | Ekran **POST** sterownika | SD: OK, RTC: OK, MCP: OK; GPS: BRAK (normalne na początku); ENK: skalibrowany |
| 5 | Naciśnij START (zamknij ekran QR), sterownik pokazuje HOME | „Gotowy" |
| 6 | Moduł 7": pasek górny | **POŁĄCZONO** (zielony). Jeśli „BRAK HASŁA" — MENU → POŁĄCZENIE WiFi, hasło z ekranu QR |
| 7 | **GPS:** poczekaj na fix (zwykle do kilku minut na zimno) | GPS *N* sat (zielony); ≥ 4 satelity |
| 8 | **Poziom farby** | zgodny ze zbiornikiem; jeśli nie — MENU → FARBA / ZBIORNIK, popraw |
| 9 | **Kalibracja — kontrola:** przejedź znany odcinek (np. 10 m taśmą) | dystans na ekranie ≈ 10,0 m (tolerancja ~1 %) |
| 10 | Jeśli wynik zły: **kalibracja 10 m** (rozdz. 8.10) | nowy wynik impulsów/metr zapisany |
| 11 | **Test pistoletów** (menu serwisowe sterownika → *Czyszczenie dysz*): wybierz wzorce i **trzymaj START** | każdy z 6 pistoletów strzela równo, bez przecieków i pluć |
| 12 | Test P1+P3 (wzorzec P-4), P2 (P-1a), P4 (P-2b), P5 (P-7c), P6 (P-7a) | wszystkie kombinacje działają |
| 13 | **Test STOP:** START malowania na postoju → STOP na ekranie 7" | pistolety OFF, stan ZATRZYMANY |
| 14 | **Test fizycznego STOP** (panel, pilot, pedał) | pistolety OFF natychmiast |
| 15 | Ustawienia: *Zmiana wzorca* (Smart/Instant), *auto-wznowienie*, progi prędkości | zgodnie z planem (rozdz. 7) |
| 16 | Tryb pracy | AUTO (lub inny wg planu) |
| 17 | Wybierz pierwszy wzorzec i sprawdź kształt na ekranie | zgodny z projektem |
| 18 | Ustawienie maszyny w punkcie startu pierwszego odcinka | patrz rozdz. 5 |
| 19 | Odprawa zespołu: kolejność prac, sygnały, punkt zakończenia, procedura STOP | wszyscy znają plan |

**Nie zaczynaj malowania**, jeśli którykolwiek z kroków 4, 6, 9, 11, 13 lub 14 dał wynik inny niż oczekiwany.

---

## 5. Rozpoznanie odcinka i plan prac

### 5.1 Zbieranie danych

1. Odczytaj z projektu: rodzaj linii (oś, krawędź), wzorzec, długości odcinków, punkty początku/końca, miejsca zmian.
2. Przejdź/przejedź odcinek: stan nawierzchni, zanieczyszczenia, wilgoć, istniejące oznakowanie, kolizje z instalacjami.
3. Oznacz na nawierzchni **punkty startu i końca** oraz punkty zmiany wzorca (kreda/farba lub taśma).
4. Zmierz długości kluczowych odcinków taśmą i porównaj z projektem oraz z pomiarem komputera.

### 5.2 Kolejność prac (wskazówki)

Kolejność zwykle wynika z projektu i organizacji ruchu. Jeśli projekt jej nie narzuca, praktyczne jest:

1. **Krawędzie** (P-6, P-7a…P-7d) — strona po stronie, po zakończeniu **przejazd bez malowania** (STOP) do początku następnej.
2. **Oś** (P-1a…P-4) — najpierw linie ciągłe/przekraczalne, potem przerywane lub odwrotnie, wg zaleceń farby i projektu.
3. **Elementy specjalne** (przejścia, strzałki, napisy) w trybie SEMI lub RĘCZNY.

Cała praca **w jednym kierunku jazdy** (jeśli to możliwe) upraszcza łączenie odcinków.

### 5.3 Podział na etapy i nazwy

Każde **STOP → START** to nowy etap i nowy raport. Planuj etapy tak, by odpowiadały odcinkom projektu, i zapisuj w dzienniku
(karta E) numer etapu, wzorzec, km/odcinek i godzinę.

---

## 6. Pogoda i podłoże

**Orientacyjne zasady** (sprawdź kartę techniczną farby — jest wiążąca):

| Warunek | Zwykle | Działanie |
|---------|--------|-----------|
| Temperatura powietrza/podłoża | farby wodorozcieńczalne często wymagają ok. ≥ 5 °C | poniżej — nie maluj lub użyj farby przewidzianej na niskie temperatury |
| Temperatura farby | komputer ostrzega poniżej **5 °C** („za zimna") i powyżej **35 °C** („za ciepła") — czujnik DS18B20 (opcja) | podgrzewać/schłodzić zbiornik, przechowywać w cieniu |
| Wilgotność, rosa | podłoże powinno być suche i zwykle kilka stopni powyżej punktu rosy | mokra nawierzchnia = nie maluj |
| Wiatr | silny wiatr znosi natrysk | osłony, zwolnienie, przerwanie prac przy zbyt silnym wietrze |
| Deszcz | farba zmyta lub nierówna | przerwij; farba musi mieć czas utwardzenia przed opadem |
| Słońce | zagrzewa farbę, utrudnia odczyt ekranu | osłona przeciwsłoneczna modułu 7"; zwiększ jasność (MENU → USTAWIENIA) |
| Kurz, zanieczyszczenie | pogarsza przyczepność | zamieść/zdmuchnij przed malowaniem |

**Ekran 7" w pełnym słońcu:** ustaw jasność na maksimum (USTAWIENIA → Jasność), użyj osłony; standardowe moduły nie są
projektowane pod bezpośrednie słońce.

---

## 7. Parametry robocze: prędkość, farba, zasięg

### 7.1 Prędkość

Komputer pracuje w zakresie **od minimalnej do maksymalnej prędkości malowania** (domyślnie **3 – 15 km/h**; nastawy 0–10 i 5–30
w MENU → USTAWIENIA). Poniżej minimalnej pistolety są wyłączone, powyżej maksymalnej — wyłączone z alarmem.

| km/h | m/min | min na 1 km |
|------|-------|-------------|
| 3 | 50 | 20,0 |
| 4 | 67 | 15,0 |
| 5 | 83 | 12,0 |
| 6 | 100 | 10,0 |
| 8 | 133 | 7,5 |
| 10 | 167 | 6,0 |
| 12 | 200 | 5,0 |
| 15 | 250 | 4,0 |

**Zalecenie:** jedź **równo**, w środku dozwolonego zakresu. Nagłe zmiany prędkości i zatrzymania psują jakość linii;
przy zatrzymaniu włączy się auto-pauza (< 0,5 km/h przez 1,5 s).

### 7.2 Zużycie farby

Komputer szacuje zużycie ze wzoru: **litry = powierzchnia [m²] × współczynnik [l/m²]** (domyślnie **0,6 l/m²**, zbiornik **200 L**;
współczynnik do ustawienia przez panel WWW / API `set_paint_rate` wg karty farby i pomiaru).
Powierzchnia = suma (szerokość pistoletu × przejechana długość) dla pistoletów, które strzelały.

### 7.3 Zasięg na jednym zbiorniku (200 L, 0,6 l/m²)

| Wzorzec | Pistolety | Powierzchnia na 1 m drogi | Zasięg na 200 L | Czas przy 5 km/h |
|---------|-----------|---------------------------|-----------------|-------------------|
| P-1a | P2 (4 m / 8 m) | 0,040 m² | ≈ 8 330 m | ≈ 100 min |
| P-1b | P2 (2 m / 4 m) | 0,040 m² | ≈ 8 330 m | ≈ 100 min |
| P-1c | P2 (2 m / 2 m) | 0,060 m² | ≈ 5 560 m | ≈ 67 min |
| P-1d | P2 (1 m / 1 m) | 0,060 m² | ≈ 5 560 m | ≈ 67 min |
| P-1e | P4 (1 m / 1 m) | 0,120 m² | ≈ 2 780 m | ≈ 33 min |
| P-2a | P2 ciągła | 0,120 m² | ≈ 2 780 m | ≈ 33 min |
| P-2b | P4 ciągła | 0,240 m² | ≈ 1 390 m | ≈ 17 min |
| P-3a | P1 ciągła + P3 (4 m / 2 m) | 0,200 m² | ≈ 1 670 m | ≈ 20 min |
| P-3b | P1 ciągła + P3 (1 m / 1 m) | 0,180 m² | ≈ 1 850 m | ≈ 22 min |
| P-4 | P1 + P3 ciągłe | 0,240 m² | ≈ 1 390 m | ≈ 17 min |
| P-6 | P5 (4 m / 2 m) | 0,080 m² | ≈ 4 170 m | ≈ 50 min |
| P-7a | P6 (1 m / 1 m) | 0,120 m² | ≈ 2 780 m | ≈ 33 min |
| P-7b | P6 ciągła | 0,240 m² | ≈ 1 390 m | ≈ 17 min |
| P-7c | P5 (1 m / 1 m) | 0,060 m² | ≈ 5 560 m | ≈ 67 min |
| P-7d | P5 ciągła | 0,120 m² | ≈ 2 780 m | ≈ 33 min |

Zasięg = 200 L / (0,6 l/m² × powierzchnia na 1 m). Dla innego zbiornika lub współczynnika przelicz proporcjonalnie.
**Przykład:** 3,4 km linii P-2a przy 5 km/h zużyje ok. 245 L (3 400 m × 0,12 m² × 0,6) i potrwa ok. 41 min — potrzebne **dwa** napełnienia.

**Zapas:** planuj farbę z rezerwą 10–15 % i **zaplanuj miejsca tankowania** na trasie (przy P-2b/P-4/P-7b zbiornik wystarcza na kilkanaście minut).

---

## 8. Procedury malowania według typu linii

Wspólne kroki przed **każdym** odcinkiem (skrót — pełna lista: karta B):

1. Maszyna ustawiona w punkcie startu, **na wyznaczonej linii**, w pozycji roboczej.
2. Poziom farby i pistolety OK; bez alarmów na ekranie.
3. Właściwy wzorzec i tryb; **strona OŚ/KRAWĘDŹ** zgodna z celem.
4. Sygnał od osoby zabezpieczającej ruch; droga wolna.
5. **START** (lub START OD PRZERWY — wg potrzeby).
6. Po **20–30 m sprawdź** na nawierzchni geometrię (szerokość, kreska/przerwa, położenie) i zatrzymaj, jeśli coś nie gra.

### 8.1 A. Linia przerywana osiowa (P-1a…P-1e)

- **Cel:** oś jezdni, np. P-1a (4 m / 8 m), P-1b (2 m / 4 m), P-1c (2 m / 2 m), P-1d (1 m / 1 m), P-1e (1 m / 1 m, 24 cm).
- Strona **OŚ**, wybierz wzorzec (P-1a…P-1e = S1…S5).
- **START** w punkcie początku. Pistolet zacznie malować po przekroczeniu prędkości minimalnej.
- **Kontrola:** długość kreski i przerwy taśmą; kapsuła P2 (lub P4) zielona w kresce, ciemna w przerwie.
- **Koniec odcinka:** dojedź do punktu końcowego, **STOP**. Jeśli linia ma być kontynuowana, zakończ po pełnej kresce
  lub po pełnej przerwie i zanotuj fazę (rozdz. 9.2).

### 8.2 B. Linia ciągła (P-2a, P-2b)

- Wzorce P-2a (12 cm, P2), P-2b (24 cm, P4).
- Prędkość równa, bez zatrzymań; **zatrzymanie na postoju → auto-pauza**, po ruszeniu i włączonym *auto-wznowieniu* wznowienie
  (z zabezpieczeniem 2 s przed ponowną pauzą).
- Zasięg zbiornika krótki (rozdz. 7.3): planuj tankowania.

### 8.3 C. Linia przekraczalna i odwracanie (P-3a, P-3b)

- P1 (ciągły) + P3 (przerywany). Domyślnie ciągła po lewej stronie maszyny.
- **Pierwszy kierunek:** bez `<>`; **drugi kierunek (po zawróceniu):** dotknij **ODWRÓĆ** — pojawi się `<>`, role P1/P3 zamienione.
- Odwrócenie można wykonać w trakcie jazdy; jednak **przełączaj na początku odcinka**, nie w środku kreski.
- **Kontrola:** po którą stronę maszyny jest linia ciągła — sprawdź na pierwszych metrach w obu kierunkach.

### 8.4 D. Linia podwójna ciągła (P-4)

- P1 + P3 ciągłe (24 cm łączna szerokość).
- Zapotrzebowanie farby wysokie (rozdz. 7.3). Odstęp między liniami wynika z mocowania pistoletów — sprawdź po montażu.

### 8.5 E. Krawędzie jezdni (P-6, P-7a…P-7d)

- Przełącz na stronę **KRAWĘDŹ** (dotknij zakładkę KRAWĘDŹ / fizyczny GRUPA). Wzorce: P-6, P-7a…P-7d, WŁASNY.
- Pistolety P5 (12 cm) i P6 (24 cm) — mocowane na krawędzi; sprawdź ich położenie względem krawędzi jezdni **przed startem**.
- Prowadzenie maszyny równolegle do krawędzi: **utrzymuj stały odstęp** (pomocnik pilnuje).
- Prawą i lewą krawędź maluj osobnymi etapami (STOP → przejazd → START), zapisując stronę w dzienniku.

### 8.6 F. Start od przerwy — kontynuacja istniejącej linii

- Zastosuj, gdy maszyna stoi w punkcie, w którym istniejąca linia przerywana ma **początek przerwy**.
- Wybierz wzorzec, ustaw maszynę **dokładnie** w punkcie, **START OD PRZERWY**. Pierwsza przerwa ma pełną długość przerwy
  wzorca, potem kreska (patrz [INSTRUKCJA_OBSLUGI.md](INSTRUKCJA_OBSLUGI.md), sekcja 7).
- Dla wzorców ciągłych nie ma różnicy względem zwykłego START.

### 8.7 G. Tryb SEMI — przejścia dla pieszych, skrzyżowania

- Wybierz **SEMI** (maszyna stoi). Wzorzec kreski wg projektu.
- START → maszyna maluje **jedną kreskę** i wyłącza pistolet (krótki sygnał). Baner „LINIA GOTOWA".
- Dojedź do miejsca następnej kreski i naciśnij **NASTĘPNA LINIA**.
- Zalety: precyzyjne pozycjonowanie każdej kreski; wada: wolniejsze tempo. Dla przejść dla pieszych pracuj **powoli**.
- **Kontrola:** numer segmentu (`SEMI #n`) w opisie trybu — porównaj z liczbą pasów w projekcie.

### 8.8 H. Tryb RĘCZNY — strzałki i symbole

- Wybierz **RĘCZNY**. START (na ekranie) rozpoczyna sesję; **pistolety strzelają tylko przy trzymanym fizycznym START**
  (panel, pilot lub pedał) i prędkości w zakresie.
- Sposób pracy: **jedna osoba prowadzi, druga trzyma START** (pedał!) w miejscach, gdzie ma być farba.
- Pistolety wyłączone we wzorcu pozostają wyłączone; dobierz wzorzec z właściwymi pistoletami.
- Puszczenie START = natychmiast OFF. Symbole rysuj na sucho po planie (szablon/naklejki lub znaczniki).

### 8.9 I. Wzorzec własny

- MENU → WZÓR WŁASNY: wybierz slot (1–3), ustaw dla każdego pistoletu WYŁ / CIĄGŁY / PRZERYW. i długości kreski/przerwy
  (0,5–50 m). **ZAPISZ I UŻYJ**.
- Test na sucho (*Czyszczenie dysz*) i **pierwsze 20–30 m** pod kontrolą geometrii.
- Zapisz w dzienniku numer slotu i konfigurację (do powtórzenia i rozliczenia).

### 8.10 J. Kalibracja w terenie (10 m)

1. Wyznacz na prostym, równym podłożu **dokładnie 10 m** (taśma stalowa, dwa punkty).
2. Maszyna stoi na punkcie początkowym (stan GOTOWY).
3. **MENU → KALIBRACJA → START KALIBRACJI** (lub menu serwisowe sterownika → *Kalibracja enkodera* → START).
4. Przejedź **po prostej** do punktu końcowego.
5. **KONIEC KALIBRACJI** (lub START na sterowniku).
6. Sprawdź wynik (impulsy/metr) i wykonaj przejazd kontrolny. **Zapisz wartość w dzienniku** (do porównań).
7. Powtórz, jeśli wynik odbiega od poprzedniego o więcej niż 1–2 % (koło, docisk, poślizg).

Kalibrację wykonuj po wymianie koła lub enkodera, przy podejrzeniu błędu dystansu oraz **przy zmianie nawierzchni** o innym poślizgu.

---

## 9. Zmiana wzorca, łączenie odcinków, przejazdy

### 9.1 Zmiana wzorca w trakcie pracy

- **Smart** (domyślne): bieżący cykl kreska+przerwa jest dokańczany; na ekranie `OCZEKUJE: P-xx (po cyklu)`.
  Stosuj, gdy nie potrzebujesz ostrej zmiany w konkretnym punkcie.
- **Instant:** zmiana natychmiast — stosuj, gdy zmiana ma nastąpić dokładnie w oznaczonym punkcie.
  Ustawienie: MENU → USTAWIENIA → *Zmiana wzorca w trakcie pracy* (na sterowniku: SELEKTOR długo na HOME lub SETUP).
- Fizyczne przyciski S1–S10 (układ soft-key) zmieniają wzorzec tak samo jak dotyk; **GRUPA** przełącza OŚ ⇄ KRAWĘDŹ.
- Zmiana **grupy** (OŚ → KRAWĘDŹ) w trakcie jazdy zwykle oznacza przejazd do innego położenia — rób to **na STOP**.

### 9.2 Łączenie odcinków (ciągłość faz kresek)

Aby kolejny odcinek linii przerywanej zaczynał się we właściwej fazie:

1. Odcinek kończ **na granicy faz**: **po pełnej kresce** (za chwilę zacznie się przerwa) albo **po pełnej przerwie**
   (za chwilę zacznie się kreska). **Zanotuj**, która to była faza, i oznacz punkt.
2. Kolejny etap: wybierz ten sam wzorzec i ustaw maszynę **dokładnie w tym punkcie**.
3. Jeśli poprzedni odcinek zakończył się **po pełnej kresce** → użyj **START OD PRZERWY** (pierwsza będzie przerwa).
   Jeśli zakończył się **po pełnej przerwie** → użyj zwykłego **START** (pierwsza będzie kreska).
4. Po 20–30 m skontroluj zgodność kresek z istniejącymi.

Zakończenie odcinka w środku kreski lub przerwy powoduje przesunięcie faz — unikaj go przy liniach, które mają być ciągłe optycznie.

### 9.3 Przejazdy między odcinkami

- **Zawsze STOP** (nie PAUZA) przed przejazdem transportowym — pistolety wyłączone, raport zapisany.
- Ustaw tryb i wzorzec **przed** dojazdem do punktu startu, na postoju.
- W stanie ZATRZYMANY pistolety są wyłączone. Przy przejeździe z podniesionymi pistoletami zabezpiecz koło pomiarowe
  przed uszkodzeniem.
- Przed transportem na przyczepie: STOP, zabezpiecz pistolety i koło pomiarowe.

---

## 10. Postoje, pauzy i praca w ruchu

| Sytuacja | Postępowanie |
|----------|--------------|
| Krótki postój w trakcie linii (zator, przeszkoda) | pauza automatyczna (< 0,5 km/h przez 1,5 s) lub PAUZA; po ruszeniu wznowienie (auto lub WZNÓW) |
| Dłuższy postój (> ok. 2–3 min) | STOP, zabezpiecz farbę (wysychanie w dyszach), rozważ przepłukanie |
| Przerwa w ruchu / czekanie na zwolnienie odcinka | STOP, notuj punkt zakończenia |
| Zator, pojazdy na jezdni | STOP; nie maluj w ruchu, jeśli projekt organizacji ruchu tego nie przewiduje |
| Zmiana kierunku jazdy | STOP, zawróć, ustaw ODWRÓĆ (P-3a/b) lub grupę, START |
| Zmiana operatora | STOP, przekaż stan (wzorzec, tryb, farba, alarmy), START po przekazaniu |

Po **pauzie** liczniki sesji zostają, po **STOP** — raport zamknięty. **START ze stanu ZATRZYMANY zaczyna nowy etap** (liczniki od zera).

---

## 11. Farba i tankowanie

### 11.1 Ocena poziomu

- Pasek FARBA w górnym pasku: pomarańczowy poniżej **15 %**, czerwony poniżej **5 %**. **Nie dopuszczaj do 5 %** — dolewaj wcześniej.
- Poziom jest szacunkowy; przy wątpliwościach sprawdź zbiornik wzrokowo.

### 11.2 Tankowanie krok po kroku

1. **STOP**, zabezpiecz maszynę (ręczny hamulec, koła), wyłącz ciśnienie zgodnie z instrukcją zbiornika.
2. Dolej farbę (zgodnie z kartą: mieszanie, rozcieńczenie, temperatura). Zanotuj **ilość** i **numer partii**.
3. **MENU → FARBA / ZBIORNIK:** wpisz **+10 / +25 / +50 / +100 L** albo **DO PEŁNA**. Sprawdź pojemność zbiornika.
4. Poziom w pasku i w oknie ma być zgodny z faktycznym stanem.
5. Zanotuj tankowanie w dzienniku (godzina, ilość, partia).

Alternatywnie tankowanie zapisujesz na sterowniku: menu serwisowe → *Tankowanie farby*.

### 11.3 Rozliczenie

- Porównaj **zużycie rzeczywiste** (farba dolana − farba pozostała) z **szacunkiem komputera**.
- Jeśli różnica > 10 %, skoryguj **współczynnik zużycia** (panel WWW / `set_paint_rate`, 0,1–5,0 l/m²) albo sprawdź szczelność/dysze.
- Statystyki farby (zużyte litry, liczba tankowań, łącznie zatankowano) są w MENU → STATYSTYKI.

---

## 12. Dokumentacja pracy

### 12.1 Co zapisuje system

| Dane | Gdzie | Kiedy |
|------|-------|-------|
| Raport CSV etapu | karta SD | po każdym STOP |
| Raport HTML sesji | karta SD (`/html_reports/`) | po każdym STOP |
| Trasa GPS (GPX, GeoJSON) | karta SD (`/tracks/`) | po każdym STOP |
| Statystyki lifetime | pamięć sterownika + eksport na SD | okresowo / na żądanie |
| Kopia ustawień NVS | karta SD (`/backup/`) | co 30 min i przy starcie |
| Poziom farby | pamięć sterownika | co 30 s malowania |

### 12.2 Co zapisuj ręcznie (dziennik — karta E)

- Data, warunki (temperatura, wiatr, wilgotność), operator, pomocnik.
- Numer etapu, odcinek (km/adres), wzorzec, tryb, kierunek, strona (lewa/prawa).
- Dystans i powierzchnia z ekranu / raportu, godziny START–STOP.
- Farba: partia, dolane litry, godziny tankowań, pozostałość.
- Uwagi: alarmy, korekty, przerwy, uszkodzenia, zdjęcia (numery).
- Wartość kalibracji (impulsy/metr).

### 12.3 Pobieranie danych

- **Telefon/laptop:** WiFi sterownika `TrassarV3` → panel WWW → Menu serwisowe → Raporty SD / Trasy / Raporty HTML.
- **Karta SD:** wyjmij po zatrzymaniu maszyny i wyłączeniu zasilania, skopiuj pliki na komputer.
- Nazwy plików mają datę i godzinę — **ustaw poprawnie zegar** (RTC).

---

## 13. Sytuacje awaryjne

Ogólna zasada: **najpierw bezpieczeństwo ludzi i ruchu, potem maszyna, potem jakość pracy.**

### 13.1 Awaryjne zatrzymanie

1. **Fizyczny STOP** (panel, pilot, pedał lub grzybek, jeśli zamontowany). Pistolety zamykają się natychmiast.
2. Zatrzymaj maszynę, włącz hamulec, ostrzeż zespół i ruch.
3. Ustal przyczynę, dopiero potem START.

### 13.2 Utrata łączności modułu 7" ze sterownikiem

- Ekran: czerwone **BRAK ŁĄCZNOŚCI ZE STEROWNIKIEM**.
- **Sterownik nadal pracuje samodzielnie** (pistolety wg wzorca). Jeśli trzeba zatrzymać — **fizyczny STOP**.
- Po chwili moduł łączy się sam. Jeśli nie: MENU → POŁĄCZENIE WiFi (hasło), zasięg, liczba klientów sterownika (max 4).
- Dalej pracujesz **z panelu fizycznego sterownika** (przyciski wzorców, START/STOP) do czasu przywrócenia ekranu.

### 13.3 „ZA SZYBKO!" (alarm prędkości)

- **Pistolety wyłączone**, buzzer 3 kHz co 2 s.
- Zwolnij poniżej progu maksymalnego. Po spadku pistolety włączą się same.
- Kontrola: fragment przejechany za szybko może mieć **brak farby** — oznacz i uzupełnij.

### 13.4 „ZA WOLNO"

- Prędkość poniżej minimalnej: pistolety wyłączone. Przyspiesz w zakres.
- Jeśli maszyna stoi: auto-pauza; po ruszeniu wznowienie (jeśli włączone).

### 13.5 „ANOMALIA PISTOLETU P*n*"

- Zgłaszana po przejechaniu **50 m** w etapie, gdy pistolet użyty we wzorcu praktycznie nie malował (< 1 m).
- **STOP.** Sprawdź: dyszę (zatkana), przewód i przekaźnik, poziom farby, zawór.
- Naprawa: *Czyszczenie dysz* (test pistoletu), wymiana dyszy/przekaźnika. Oznacz i **domaluj** przejechany fragment.

### 13.6 Zatkana dysza w trakcie pracy

1. **STOP**, zabezpiecz ciśnienie.
2. Wyczyść/wymień dyszę, wykonaj *Czyszczenie dysz* (test).
3. Wróć do punktu ostatniej dobrej farby i wznów (**START** lub **START OD PRZERWY** wg fazy).
4. Domaluj brakujący odcinek.

### 13.7 Wyciek farby / przeciek

- **STOP**, zamknij zawór/odetnij zasilanie farby, powstrzymaj rozlew (sorbent), poinformuj odpowiedzialnego.
- Nie uruchamiaj, dopóki nie zlokalizujesz nieszczelności.

### 13.8 Zanik zasilania / restart sterownika

- Przy zaniku zasilania i po resecie **pistolety wyłączają się sprzętowo** (handler przed resetem, watchdog 5 s).
- Po włączeniu: stan **GOTOWY**; **nie zakładaj**, że raport przerwanego etapu został zapisany — zanotuj dystans/pozycję ręcznie.
- Wznowienie: sprawdź wzorzec i tryb, ustaw maszynę w punkcie przerwania, **START** lub **START OD PRZERWY**.
- Sprawdź poziom farby (zapisywany co 30 s) i kalibrację; pobierz log zdarzeń z SD, jeśli restarty się powtarzają.

### 13.9 Brak GPS

- Malowanie działa normalnie; brak zapisu współrzędnych w raporcie. Odsuń antenę od metalu, wynieś na zewnątrz kabiny.
- Jeśli lokalizacja jest wymagana przez zleceniodawcę — poczekaj na fix przed startem odcinka.

### 13.10 Problemy z enkoderem

- **Brak impulsów przy jeździe:** auto-pauza po 5 s bez impulsów. Sprawdź koło, kabel, złącze J3, docisk.
- **Zła długość:** ponów kalibrację 10 m; sprawdź poślizg koła na nawierzchni.

### 13.11 Deszcz / nagła zmiana pogody

- **STOP**, zabezpiecz farbę przed wodą, przepłucz układ według instrukcji, zabezpiecz moduł 7" i złącza.
- Zanotuj punkt przerwania i stan świeżo namalowanego odcinka (ochrona/oznakowanie do czasu utwardzenia).

### 13.12 Zagrożenie ruchu / kolizja

- **STOP** (fizyczny), zabezpiecz miejsce, wezwij pomoc, ostrzeż ruch.
- Nie uruchamiaj pracy do zakończenia czynności i decyzji odpowiedzialnego.

### 13.13 Pożar / rozlanie substancji

- **STOP i odłączenie zasilania**, ewakuacja zgodnie z procedurą, gaśnica proszkowa/CO₂, alarm 112.
- Nie gaś wodą urządzeń pod napięciem 12/24 V.

---

## 14. Zakończenie zmiany

1. Dokończ ostatni odcinek, **STOP**. Zapisz podsumowanie etapu (SUMMARY).
2. **Zapisz** dystans, powierzchnię i farbę w dzienniku; skorzystaj z MENU → STATYSTYKI.
3. **Przepłucz** układ farby i pistolety płynem zgodnie z instrukcją producenta; wykonaj *Czyszczenie dysz* w celu sprawdzenia.
4. Zdejmij dysze, wyczyść, zabezpiecz pistolety (wysychająca farba niszczy dysze).
5. **Sprawdź raporty:** liczba raportów (STATYSTYKI → karta SD), spójność z dziennikiem.
6. Wyłącz sterownik i moduł 7"; odłącz zasilanie.
7. **Kopia karty SD:** wyjmij kartę i skopiuj wszystkie pliki (raporty, trasy, kopia NVS) na komputer; włóż kartę z powrotem.
8. Zabezpiecz maszynę: hamulec, przykrycie modułu 7", zamknięcie zbiornika, ładowanie akumulatora.
9. Zanotuj usterki i potrzebne części na następny dzień.

---

## 15. Konserwacja

### 15.1 Codziennie

- Oględziny kabli, złączy J1–J5, wycieków, mocowania koła pomiarowego.
- Czyszczenie dysz; kontrola szczelności; smarowanie ruchomych części wg instrukcji maszyny.
- Test STOP (panel, pilot, pedał) i *Czyszczenie dysz*.
- Czyszczenie ekranu 7" i wyświetlacza sterownika (miękka szmatka, bez rozpuszczalników).

### 15.2 Co tydzień

- Kontrola stanu przekaźników (kliknięcia, nagrzewanie), zacisków i przewodów zaworów.
- Sprawdzenie kalibracji na odcinku kontrolnym; zapis wartości.
- Sprawdzenie karty SD (pojemność, błędy), kopia archiwalna raportów.
- Kontrola antena GPS (mocowanie, kabel).

### 15.3 Co miesiąc

- Pomiar zasilania pod obciążeniem (5 V: 4,75–5,25 V przy 6 pistoletach); test spadków napięcia.
- Przegląd liczników strzałów pistoletów (STATYSTYKI) — planowanie wymiany dysz/zaworów.
- Kontrola uszczelnień wtyków, wymiana zużytych końcówek.
- Aktualizacja firmware (jeśli dostępna) przy zatrzymanej maszynie.

### 15.4 Okresowo

- Wymiana baterii CR2032 w RTC (co 2–3 lata).
- Wymiana koła pomiarowego i enkodera przy zużyciu (potem **kalibracja**).
- Przegląd instalacji elektrycznej i zabezpieczeń (PTC, TVS, bezpieczniki).

---

## 16. Usterki terenowe — tabela szybkiej diagnozy

| Objaw | Sprawdź najpierw | Działanie |
|-------|------------------|-----------|
| Moduł 7": „BRAK HASŁA" | ekran QR sterownika | MENU → POŁĄCZENIE WiFi, hasło |
| Moduł 7": „ŁĄCZENIE..." bez końca | hasło, zasięg, klienci sterownika | popraw hasło, zbliż się, odłącz telefony |
| Moduł 7": „BRAK DANYCH" | czy sterownik działa | restart sterownika |
| Moduł 7": ciemny/niewidoczny ekran w słońcu | jasność, osłona | jasność 100 %, osłona przeciwsłoneczna |
| Dotyk nie reaguje | brudny ekran, rękawica, zasilanie | oczyść, zdejmij mokrą rękawicę, sprawdź 5 V |
| Pistolet nie strzela | dysza, przekaźnik, przewód, farba | *Czyszczenie dysz*; wymień element |
| Wszystkie pistolety nie strzelają | prędkość < min, tryb RĘCZNY, zasilanie zaworów | sprawdź ekran, START fizyczny, 12/24 V na J2 COM |
| Tryb RĘCZNY: brak strzału | fizyczny START niewciśnięty | trzymaj START na sterowniku/pilocie/pedale |
| Kreski za krótkie/długie | kalibracja | kalibracja 10 m |
| Dystans się nie zgadza | kalibracja, poślizg koła | kalibracja, docisk koła |
| Prędkość zawsze 0 | enkoder, kabel J3 | sprawdź CLK/DT, docisk, wiązkę |
| Auto-pauza w trakcie jazdy | brak impulsów enkodera | sprawdź enkoder i kabel |
| „ZA SZYBKO" przy normalnej jeździe | próg za niski | USTAWIENIA → maks. prędkość |
| Wzorzec się nie zmienia | tryb Smart (czeka na koniec cyklu), zły wzorzec WŁASNY | poczekaj lub przełącz na Instant; zapisz WŁASNY |
| Przycisk fizyczny wzorca nie działa | układ przycisków, MCP23017, grupa | sprawdź układ (soft-key/klasyczny), okablowanie MCP |
| Raportu brak | SD, brak STOP | włóż kartę, wykonaj STOP |
| Zegar pokazuje złą datę | bateria RTC | wymień CR2032, ustaw czas |
| Restart w pętli | piny 26–37, joystick przy starcie | odłącz przewody z tych pinów; nie wciskaj joysticka |

---

## 17. Karty do wydruku

### Karta A — poranna (uruchomienie)

```
Data: ____________  Operator: ____________  Pomocnik: ____________
[ ] Oględziny maszyny, wycieki, kable J1-J5, koło pomiarowe
[ ] Zasilanie 5 V (4,75-5,25 V):  ______ V
[ ] Sterownik: POST  SD [ ]  RTC [ ]  MCP [ ]  ENK [ ]
[ ] Moduł 7": POŁĄCZONO
[ ] GPS fix, satelity: ______
[ ] Poziom farby: ______ L (____ %)   zgodny ze zbiornikiem [ ]
[ ] Kalibracja sprawdzona:  ______ m z 10,0 m   imp/m: ______
[ ] Test 6 pistoletów (Czyszczenie dysz)   P1[ ] P2[ ] P3[ ] P4[ ] P5[ ] P6[ ]
[ ] Test STOP (ekran 7")   [ ]      Test STOP fizyczny (panel/pilot/pedał)   [ ]
[ ] Ustawienia: Smart/Instant ___  auto-wznowienie ___  min ___  max ___ km/h
[ ] Odprawa zespołu, projekt organizacji ruchu, sygnały
```

### Karta B — przed każdym odcinkiem

```
Odcinek / km: ________________  Etap nr: ____  Kierunek: ________  Strona: L / P / oś
[ ] Strona OŚ / KRAWĘDŹ zgodna z celem     Wzorzec: ________   Tryb: AUTO / SEMI / RĘCZNY
[ ] Odwrócenie (P-3a/b):  tak / nie
[ ] Start od przerwy:  tak / nie   (poprzedni odcinek zakończony: po kresce -> od przerwy; po przerwie -> zwykły START)
[ ] Poziom farby wystarcza na odcinek?  ______ L potrzebne / ______ L jest
[ ] Punkt startu oznaczony, maszyna w linii, droga wolna
[ ] Kontrola geometrii po 20-30 m:   szerokość [ ]  kreska/przerwa [ ]  położenie [ ]
```

### Karta C — po odcinku

```
Godzina START: ______  STOP: ______     Dystans: ______ m    Powierzchnia: ______ m2
Alarmy: ZA SZYBKO [ ]  ZA WOLNO [ ]  ANOMALIA [ ]   uwagi: _____________________
Koniec odcinka:  po pełnej kresce [ ]  po pełnej przerwie [ ]  (do łączenia)     Raport zapisany [ ]
Domalowanie potrzebne:  tak / nie   gdzie: ______________________
```

### Karta D — koniec dnia

```
[ ] Ostatni STOP i podsumowanie etapu zapisane
[ ] Przepłukanie układu i pistoletów, dysze zdjęte i wyczyszczone
[ ] Liczba raportów na SD: ____   Kopia karty SD wykonana [ ]
[ ] Zasilanie odłączone, maszyna zabezpieczona
[ ] Usterki / części potrzebne: ______________________________
[ ] Farba: zużyto ______ L  /  zostało ______ L
```

### Karta E — dziennik prac (tabela)

| Etap | Odcinek/km | Wzorzec | Tryb | Strona/kier. | START | STOP | Dystans [m] | Pow. [m²] | Farba [L] | Uwagi |
|------|-----------|---------|------|---------------|-------|------|-------------|-----------|-----------|-------|
|      |           |         |      |               |       |      |             |           |           |       |
|      |           |         |      |               |       |      |             |           |           |       |
|      |           |         |      |               |       |      |             |           |           |       |
|      |           |         |      |               |       |      |             |           |           |       |

**Tankowania:** godzina ____ ilość ____ L partia ______ | godzina ____ ilość ____ L partia ______

### Karta F — protokół awarii

```
Data/godzina: ______   Miejsce: ______   Operator: ______
Zdarzenie: [ ] utrata łączności  [ ] ZA SZYBKO  [ ] anomalia pistoletu P__  [ ] restart  [ ] wyciek  [ ] inne: ______
Objawy na ekranie / buzzer: _________________________________________
Działania (kolejność): ______________________________________________
Przyczyna: ________________________   Naprawa: ________________________
Wpływ na jakość / fragment do domalowania: _________________________
Zdjęcia nr: ______     Zgłoszono do: ______
```

---

## 18. Ściągawka

### Moduł 7" — ekran roboczy

| Chcę… | Robię… |
|-------|--------|
| wybrać wzorzec osi | zakładka **OŚ**, dotknij S1–S10 |
| wybrać wzorzec krawędzi | zakładka **KRAWĘDŹ**, dotknij S1–S6 |
| wybrać dowolny wzorzec | MENU → WSZYSTKIE WZORCE |
| odwrócić P-3a/b | **ODWRÓĆ** na drodze |
| zmienić tryb | (maszyna stoi) **AUTO / SEMI / RĘCZNY** |
| zacząć | **START** (zielony); od przerwy — **START OD PRZERWY** |
| zatrzymać | **STOP** (czerwony) lub fizyczny STOP |
| następna kreska (SEMI) | **NASTĘPNA LINIA** |
| tankowanie | MENU → FARBA / ZBIORNIK |
| kalibracja | MENU → KALIBRACJA |
| progi prędkości, Smart/Instant | MENU → USTAWIENIA |
| hasło WiFi | MENU → POŁĄCZENIE WiFi |
| statystyki | MENU → STATYSTYKI |

### Sterownik — panel fizyczny

| Przycisk | Krótko | Długo (1,5 s) |
|----------|--------|----------------|
| START | start / pauza / wznów; SEMI: następna linia; RĘCZNY: **trzymaj = strzelaj** | HOME: SETUP |
| STOP | zatrzymaj; w menu wstecz | HOME: menu serwisowe |
| SELEKTOR | odwróć P-3a/b; w menu dalej | HOME: Smart/Instant |
| GAP | start od przerwy | — |
| S1–S10, GRUPA | wzorce wg układu soft-key (GRUPA: OŚ ⇄ KRAWĘDŹ) | — |

### Kolory kapsuł pistoletów

- **Zielona** — strzela · **żółta ramka** — użyty we wzorcu · **ciemna** — nieużywany · **czerwona ramka** — anomalia.

### Sygnały dźwiękowe

- 1 krótki 2 kHz — start/wznów · 2 krótkie — pauza/stop · 2 pulsy 1,5 kHz — za wolno · 3 szybkie 3 kHz — za szybko ·
  niski-wysoki-niski — anomalia pistoletu · opadający — błąd startowy SD/RTC.

---

*Instrukcja terenowa MPD2026. Wartości orientacyjne (pogoda, zasięgi farby) należy weryfikować z kartami producentów i
wymaganiami zleceniodawcy.*

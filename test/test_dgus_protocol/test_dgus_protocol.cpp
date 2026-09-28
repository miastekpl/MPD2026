// ============================================================
// MPD2026 - testy protokolu DWIN DGUS (shared/dgus_protocol.h)
// pio test -e native
// ============================================================

#include <unity.h>
#include <cstring>
#include <vector>

#include "dgus_protocol.h"

using namespace dgus;

struct Frame { uint8_t cmd; uint16_t vp; std::vector<uint16_t> words; };

static std::vector<Frame> feedAll(Parser& p, const uint8_t* data, size_t n) {
    std::vector<Frame> out;
    for (size_t i = 0; i < n; i++) {
        if (p.feed(data[i])) {
            Frame f;
            f.cmd = p.cmd();
            f.vp = p.vp();
            for (size_t w = 0; w < p.wordCount(); w++) f.words.push_back(p.word(w));
            out.push_back(f);
        }
    }
    return out;
}

void setUp() {}
void tearDown() {}

void test_write_single_word_matches_spec_example() {
    // Z dokumentacji DWIN: "Write the value 1234 in VP 0x1000" -> 5A A5 05 82 1000 04D2
    uint8_t frame[dgus::MAX_FRAME];
    size_t n = encodeWriteWord(0x1000, 0x04D2, frame, sizeof(frame));
    TEST_ASSERT_EQUAL_UINT32(8, n);
    const uint8_t expected[] = {0x5A, 0xA5, 0x05, 0x82, 0x10, 0x00, 0x04, 0xD2};
    TEST_ASSERT_EQUAL_UINT32(0, memcmp(frame, expected, n));
}

void test_write_multi_word_matches_spec_example() {
    // "Write values on 4 sequential VPs, starting from VP 0x1000" -> 5A A5 0B 82 1000 0022 0071 0006 0031
    uint16_t words[4] = {0x0022, 0x0071, 0x0006, 0x0031};
    uint8_t frame[dgus::MAX_FRAME];
    size_t n = encodeWrite(0x1000, words, 4, frame, sizeof(frame));
    const uint8_t expected[] = {0x5A, 0xA5, 0x0B, 0x82, 0x10, 0x00,
                                 0x00, 0x22, 0x00, 0x71, 0x00, 0x06, 0x00, 0x31};
    TEST_ASSERT_EQUAL_UINT32(sizeof(expected), n);
    TEST_ASSERT_EQUAL_UINT32(0, memcmp(frame, expected, n));
}

void test_read_request_matches_spec_example() {
    // "Read the value in VP 0x1000" -> 5A A5 04 83 1000 01
    uint8_t frame[dgus::MAX_FRAME];
    size_t n = encodeReadRequest(0x1000, 1, frame, sizeof(frame));
    const uint8_t expected[] = {0x5A, 0xA5, 0x04, 0x83, 0x10, 0x00, 0x01};
    TEST_ASSERT_EQUAL_UINT32(sizeof(expected), n);
    TEST_ASSERT_EQUAL_UINT32(0, memcmp(frame, expected, n));
}

void test_parse_read_response_matches_spec_example() {
    // "Answer from LCM" dla odczytu VP 0x1000 -> 5A A5 06 83 1000 01 04D2
    const uint8_t wire[] = {0x5A, 0xA5, 0x06, 0x83, 0x10, 0x00, 0x01, 0x04, 0xD2};
    Parser p;
    auto frames = feedAll(p, wire, sizeof(wire));
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_EQUAL_UINT32(CMD_READ, frames[0].cmd);
    TEST_ASSERT_EQUAL_UINT32(0x1000, frames[0].vp);
    TEST_ASSERT_EQUAL_UINT32(1, frames[0].words.size());
    TEST_ASSERT_EQUAL_UINT32(0x04D2, frames[0].words[0]);
}

void test_parse_multiword_response() {
    // "Read values on 4 sequential VPs" -> 5AA5 0C 83 0000 04 0022 0071 0006 0031
    const uint8_t wire[] = {0x5A, 0xA5, 0x0C, 0x83, 0x00, 0x00, 0x04,
                             0x00, 0x22, 0x00, 0x71, 0x00, 0x06, 0x00, 0x31};
    Parser p;
    auto frames = feedAll(p, wire, sizeof(wire));
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_EQUAL_UINT32(4, frames[0].words.size());
    TEST_ASSERT_EQUAL_UINT32(0x0031, frames[0].words[3]);
}

void test_roundtrip_write_then_parse() {
    uint16_t words[3] = {1, 2, 3};
    uint8_t frame[dgus::MAX_FRAME];
    size_t n = encodeWrite(0x3000, words, 3, frame, sizeof(frame));
    Parser p;
    auto frames = feedAll(p, frame, n);
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_EQUAL_UINT32(CMD_WRITE, frames[0].cmd);
    TEST_ASSERT_EQUAL_UINT32(0x3000, frames[0].vp);
    TEST_ASSERT_EQUAL_UINT32(3, frames[0].words.size());
    TEST_ASSERT_EQUAL_UINT32(2, frames[0].words[1]);
}

void test_touch_push_frame_same_shape_as_read_response() {
    // Ekran po dotknieciu klawisza (skonfigurowanego na zapis pod VP rejestru zdarzen)
    // wysyla NIEZADANA ramke w formacie identycznym z odpowiedzia na odczyt.
    const uint8_t wire[] = {0x5A, 0xA5, 0x06, 0x83, 0x30, 0x00, 0x01, 0x00, 0x07};  // VP 0x3000 = 7 (EVT_GAP_START)
    Parser p;
    auto frames = feedAll(p, wire, sizeof(wire));
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_EQUAL_UINT32(0x3000, frames[0].vp);
    TEST_ASSERT_EQUAL_UINT32(7, frames[0].words[0]);
}

void test_text_field_pads_and_truncates() {
    uint8_t frame[dgus::MAX_FRAME];
    size_t n = encodeWriteText(0x2000, "AB", 3, frame, sizeof(frame));  // 3 slowa = 6 bajtow pola
    TEST_ASSERT_TRUE(n > 0);
    Parser p;
    auto frames = feedAll(p, frame, n);
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_EQUAL_UINT32(3, frames[0].words.size());
    TEST_ASSERT_EQUAL_HEX16(0x4142, frames[0].words[0]);  // 'A','B'
    TEST_ASSERT_EQUAL_HEX16(0x0000, frames[0].words[1]);  // dopelnienie
    TEST_ASSERT_EQUAL_HEX16(0x0000, frames[0].words[2]);

    // Dluzszy tekst nadpisujacy krotszy poprzedni - zero resztek
    n = encodeWriteText(0x2000, "HELLO!", 3, frame, sizeof(frame));
    Parser p2;
    frames = feedAll(p2, frame, n);
    TEST_ASSERT_EQUAL_HEX16(0x4845, frames[0].words[0]);  // 'H','E'
    TEST_ASSERT_EQUAL_HEX16(0x4C4C, frames[0].words[1]);  // 'L','L'
    TEST_ASSERT_EQUAL_HEX16(0x4F21, frames[0].words[2]);  // 'O','!'
}

void test_page_switch_frame() {
    uint8_t frame[dgus::MAX_FRAME];
    size_t n = encodeSwitchPage(5, frame, sizeof(frame));
    const uint8_t expected[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x05};
    TEST_ASSERT_EQUAL_UINT32(sizeof(expected), n);
    TEST_ASSERT_EQUAL_UINT32(0, memcmp(frame, expected, n));
}

void test_resync_after_garbage() {
    std::vector<uint8_t> stream = {0x11, 0x22, 0x5A, 0x33};  // 5A nie nastepuje po nim A5 -> ignoruj
    uint8_t good[dgus::MAX_FRAME];
    size_t n = encodeWriteWord(0x0001, 0x0002, good, sizeof(good));
    stream.insert(stream.end(), good, good + n);
    Parser p;
    auto frames = feedAll(p, stream.data(), stream.size());
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_EQUAL_UINT32(0x0001, frames[0].vp);
}

void test_rejects_absurd_byte_count() {
    // BC nieprawdopodobnie duzy - parser ma sie wybudzic bez zawieszenia i zliczyc overflow
    std::vector<uint8_t> stream = {0x5A, 0xA5, 0xFE};
    uint8_t good[dgus::MAX_FRAME];
    size_t n = encodeWriteWord(0x0009, 0x0009, good, sizeof(good));
    stream.insert(stream.end(), good, good + n);
    Parser p;
    auto frames = feedAll(p, stream.data(), stream.size());
    TEST_ASSERT_EQUAL_UINT32(1, frames.size());
    TEST_ASSERT_TRUE(p.overflows() >= 1);
}

void test_back_to_back_frames() {
    Parser p;
    std::vector<uint8_t> stream;
    for (uint16_t i = 0; i < 15; i++) {
        uint8_t f[dgus::MAX_FRAME];
        size_t n = encodeWriteWord((uint16_t)(0x4000 + i), i, f, sizeof(f));
        stream.insert(stream.end(), f, f + n);
    }
    auto frames = feedAll(p, stream.data(), stream.size());
    TEST_ASSERT_EQUAL_UINT32(15, frames.size());
    TEST_ASSERT_EQUAL_UINT32(14, frames[14].words[0]);
    TEST_ASSERT_EQUAL_UINT32(0, p.badFrames());
}

void test_limits() {
    uint8_t frame[16];   // za maly bufor
    uint16_t words[4] = {1, 2, 3, 4};
    TEST_ASSERT_EQUAL_UINT32(0, encodeWrite(0x1000, words, 4, frame, 4));
    TEST_ASSERT_EQUAL_UINT32(0, encodeWrite(0x1000, nullptr, 4, frame, sizeof(frame)));
    TEST_ASSERT_EQUAL_UINT32(0, encodeWrite(0x1000, words, 0, frame, sizeof(frame)));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_write_single_word_matches_spec_example);
    RUN_TEST(test_write_multi_word_matches_spec_example);
    RUN_TEST(test_read_request_matches_spec_example);
    RUN_TEST(test_parse_read_response_matches_spec_example);
    RUN_TEST(test_parse_multiword_response);
    RUN_TEST(test_roundtrip_write_then_parse);
    RUN_TEST(test_touch_push_frame_same_shape_as_read_response);
    RUN_TEST(test_text_field_pads_and_truncates);
    RUN_TEST(test_page_switch_frame);
    RUN_TEST(test_resync_after_garbage);
    RUN_TEST(test_rejects_absurd_byte_count);
    RUN_TEST(test_back_to_back_frames);
    RUN_TEST(test_limits);
    return UNITY_END();
}

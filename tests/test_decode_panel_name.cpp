/*
 * Standalone unit test for decode_panel_name() (CP437 → UTF-8).
 *
 * Compile:  g++ -std=c++17 -o test_decode_panel_name tests/test_decode_panel_name.cpp && ./test_decode_panel_name
 */

// Provide the esphome namespace stubs so cp437.h compiles stand-alone.
namespace esphome {
namespace bentel_kyo {}
}  // namespace esphome

#include "../components/bentel_kyo/cp437.h"

#include <cassert>
#include <cstdio>
#include <cstring>

using esphome::bentel_kyo::decode_panel_name;

static int tests_run = 0;
static int tests_passed = 0;

#define EXPECT_EQ(actual, expected)                                                        \
  do {                                                                                     \
    tests_run++;                                                                           \
    if ((actual) == (expected)) {                                                           \
      tests_passed++;                                                                      \
    } else {                                                                               \
      std::fprintf(stderr, "FAIL %s:%d: expected \"%s\", got \"%s\"\n", __FILE__, __LINE__, \
                   std::string(expected).c_str(), std::string(actual).c_str());             \
    }                                                                                      \
  } while (0)

// Helper: build a uint8_t buffer from a C string literal (may contain embedded zeros).
static std::string decode(const uint8_t *buf, int len) { return decode_panel_name(buf, len); }

int main() {
  // --- Pure ASCII names (must pass through unchanged) ---
  {
    const uint8_t buf[] = {'O', 'u', 't', 'p', 'u', 't', ' ', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "Output 1");
  }
  {
    const uint8_t buf[] = {'Z', 'o', 'n', 'e', ' ', '0', '1', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "Zone 01");
  }
  {
    // No trailing spaces — nothing to trim.
    const uint8_t buf[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'};
    EXPECT_EQ(decode(buf, 16), "ABCDEFGHIJKLMNOP");
  }

  // --- Trailing space trimming ---
  {
    const uint8_t buf[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "");
  }
  {
    const uint8_t buf[] = {'A', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "A");
  }

  // --- Control characters are dropped ---
  {
    const uint8_t buf[] = {0x00, 0x01, 0x1F, 'H', 'e', 'l', 'l', 'o', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "Hello");
  }
  {
    // All control characters → empty.
    const uint8_t buf[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    EXPECT_EQ(decode(buf, 16), "");
  }

  // --- CP437 high-byte decoding ---
  {
    // 0x82 = é (U+00E9) → UTF-8 C3 A9
    const uint8_t buf[] = {'c', 'a', 'f', 0x82, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "caf\xC3\xA9");  // "café"
  }
  {
    // Italian accented vowels commonly found on Bentel panels.
    // à=0x85  è=0x8A  ì=0x8D  ò=0x95  ù=0x97
    const uint8_t buf[] = {0x85, 0x8A, 0x8D, 0x95, 0x97, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "\xC3\xA0\xC3\xA8\xC3\xAC\xC3\xB2\xC3\xB9");  // "àèìòù"
  }
  {
    // 0xB3 = │ (U+2502) → UTF-8 E2 94 82
    // 0xB4 = ┤ (U+2524) → UTF-8 E2 94 A4
    const uint8_t buf[] = {0xB3, 0xB4, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "\xE2\x94\x82\xE2\x94\xA4");  // "│┤"
  }

  // --- Real-world example from the panel log ---
  {
    // "7Telephone numb." — all printable ASCII, no change expected.
    const uint8_t buf[] = {0x37, 0x54, 0x65, 0x6C, 0x65, 0x70, 0x68, 0x6F,
                           0x6E, 0x65, 0x20, 0x6E, 0x75, 0x6D, 0x62, 0x2E};
    EXPECT_EQ(decode(buf, 16), "7Telephone numb.");
  }
  {
    // " Syst. Troubles\xB3" — trailing 0xB3 becomes UTF-8 │
    const uint8_t buf[] = {0x20, 0x53, 0x79, 0x73, 0x74, 0x2E, 0x20, 0x54,
                           0x72, 0x6F, 0x75, 0x62, 0x6C, 0x65, 0x73, 0xB3};
    EXPECT_EQ(decode(buf, 16), " Syst. Troubles\xE2\x94\x82");  // " Syst. Troubles│"
  }
  {
    // "\xB4Mains Fault    " — leading 0xB4 becomes UTF-8 ┤
    const uint8_t buf[] = {0xB4, 0x4D, 0x61, 0x69, 0x6E, 0x73, 0x20, 0x46,
                           0x61, 0x75, 0x6C, 0x74, 0x20, 0x20, 0x20, 0x20};
    EXPECT_EQ(decode(buf, 16), "\xE2\x94\xA4Mains Fault");  // "┤Mains Fault"
  }

  // --- Edge cases ---
  {
    // Zero-length input.
    EXPECT_EQ(decode(nullptr, 0), "");
  }
  {
    // Single printable byte.
    const uint8_t buf[] = {'X'};
    EXPECT_EQ(decode(buf, 1), "X");
  }
  {
    // Single high byte: 0xF8 = ° (U+00B0) → UTF-8 C2 B0
    const uint8_t buf[] = {0xF8};
    EXPECT_EQ(decode(buf, 1), "\xC2\xB0");  // "°"
  }
  {
    // 0x9E = ₧ (U+20A7) — a 3-byte UTF-8 code point: E2 82 A7
    const uint8_t buf[] = {0x9E, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(decode(buf, 16), "\xE2\x82\xA7");  // "₧"
  }
  {
    // Mixed: ASCII + control + CP437
    // 'A' 0x03 0x82 'B' → "AéB"
    const uint8_t buf[] = {'A', 0x03, 0x82, 'B'};
    EXPECT_EQ(decode(buf, 4), "A\xC3\xA9""B");  // "AéB"
  }

  std::printf("%d/%d tests passed\n", tests_passed, tests_run);
  return tests_passed == tests_run ? 0 : 1;
}

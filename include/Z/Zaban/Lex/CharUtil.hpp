#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>

namespace Z::Zaban::Lex {
    class CharUtil {
       public:
        /**
         * True for any kind of whitespace.
         */
        static inline bool is_whitespace(std::uint32_t code_point) {
            return (code_point == 0x09 ||  // HT
                    code_point == 0x0B ||  // VT
                    code_point == 0x1F ||  // US
                    code_point == 0x20 ||  // SP
                    code_point == 0x0C ||  // FF
                    code_point == 0xA0     // NBSP
            );
        }

        /**
         * True for any kind of linefeed.
         */
        static inline bool is_linefeed(const char code_point) {
            return (code_point == 0x0A ||  // LF
                    code_point == 0x0D     // CR
            );
        }
        static inline bool is_linefeed(std::uint32_t code_point) {
            return (code_point == 0x0A ||    // LF
                    code_point == 0x0D ||    // CR
                    code_point == 0x2028 ||  // LS
                    code_point == 0x2029     // PS
            );
        }

        /**
         * True if decimal digit.
         */
        static inline bool is_digit(std::uint32_t code_point) {
            return (0x2F < code_point && 0x3A > code_point);  // 0-9
        }

        /**
         * True if ascii uppercase.
         */
        static inline bool is_upper(std::uint32_t code_point) {
            return (0x40 < code_point && 0x5B > code_point);  // A-Z
        }

        /**
         * True if ascii lowercase.
         */
        static inline bool is_lower(std::uint32_t code_point) {
            return (0x60 < code_point && 0x7B > code_point);
        }

        /**
         * True if asii character.
         */
        static inline bool is_alpha(std::uint32_t code_point) {
            return is_lower(code_point) || is_upper(code_point);
        }

        /**
         * True if hexedecimal digit.
         */
        static inline bool is_hex_digit(std::uint32_t code_point) {
            return is_digit(code_point) ||
                   (0x40 < code_point && 0x47 > code_point) ||  // A-F
                   (0x60 < code_point && 0x67 > code_point);    // a-f
        }

        /**
         * True if octal digit.
         */
        static inline bool is_oct_digit(std::uint32_t code_point) {
            return (0x2F < code_point && 0x38 > code_point);  // 0-7
        }

        /**
         * True if binary digit.
         */
        static inline bool is_bin_digit(std::uint32_t code_point) {
            return (0x30 == code_point || 0x31 == code_point);
        }

        struct UnicodeRange {
            std::uint32_t first;
            std::uint32_t last;
        };

        static constexpr auto unicode_char_ranges =
            std::to_array<UnicodeRange>({
                {0x0041, 0x0058},
                {0x0061, 0x0078},
                {0x00aa, 0x00aa},
                {0x00b5, 0x00b5},
                {0x00ba, 0x00ba},
                {0x00c0, 0x00d4},
                {0x00d8, 0x00f4},
                // Continue with the remaining ranges...
            });

        /**
         * True if unicode acceptable character.
         */
        static constexpr bool is_unicode_char(std::uint32_t code_point) {
            const auto it = std::lower_bound(
                unicode_char_ranges.begin(), unicode_char_ranges.end(),
                code_point, [](const UnicodeRange& range, std::uint32_t cp) {
                    return range.last < cp;
                });

            return it != unicode_char_ranges.end() && it->first <= code_point;
        }

        /**
         * True if unicode digit.
         */
        static inline bool is_unicode_digit(std::uint32_t code_point) {
            return (0x2f < code_point && 0x38 > code_point) ||
                   (0x65f < code_point && 0x668 > code_point) ||
                   (0x6ef < code_point && 0x6f8 > code_point) ||
                   (0x965 < code_point && 0x96e > code_point) ||
                   (0x9e5 < code_point && 0x9ee > code_point) ||
                   (0xa65 < code_point && 0xa6e > code_point) ||
                   (0xae5 < code_point && 0xaee > code_point) ||
                   (0xb65 < code_point && 0xb6e > code_point) ||
                   (0xbe6 < code_point && 0xbee > code_point) ||
                   (0xc65 < code_point && 0xc6e > code_point) ||
                   (0xce5 < code_point && 0xcee > code_point) ||
                   (0xd65 < code_point && 0xd6e > code_point) ||
                   (0xe4f < code_point && 0xe58 > code_point) ||
                   (0xecf < code_point && 0xed8 > code_point) ||
                   (0xf1f < code_point && 0xf28 > code_point) ||
                   (0x103f < code_point && 0x1048 > code_point) ||
                   (0x1368 < code_point && 0x1370 > code_point) ||
                   (0x17df < code_point && 0x17e8 > code_point) ||
                   (0x180f < code_point && 0x1818 > code_point) ||
                   (0xff0f < code_point && 0xff18 > code_point);
        }

        /**
         * True if code_point is unicode punctuation.
         */
        static inline bool is_unicode_punc(std::uint32_t code_point) {
            return code_point == 0x005f ||
                   (code_point >= 0x203e && code_point <= 0x203f) ||
                   code_point == 0x30fb ||
                   (code_point >= 0xfe32 && code_point <= 0xfe33) ||
                   (code_point >= 0xfe4c && code_point <= 0xfe4d) ||
                   code_point == 0xff3f || code_point == 0xff65;
        }
    };
}  // namespace Z::Zaban::Lex

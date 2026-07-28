#pragma once

#include <Z/Zaban/CharUtil.hpp>
#include <cstdlib>

namespace Z::Zaban {
    class ScanUtil {
       public:
        const static inline bool is_newline_seq(const char   first,
                                                const char   second,
                                                std::size_t* count) {
            *count = 0;
            if (first == '\r' && second == '\n') {
                *count = 2;
                return true;
            }

            if (CharUtil::is_linefeed(first)) {
                *count = 1;
                return true;
            }

            return false;
        }

        const static inline bool is_newline_seq(std::uint32_t first,
                                                std::uint32_t second,
                                                std::size_t*  count) {
            *count = 0;
            if (first == '\r' && second == '\n') {
                *count = 2;
                return true;
            }

            if (CharUtil::is_linefeed(first)) {
                *count = 1;
                return true;
            }

            return false;
        }

        const static inline bool is_either_slash_comment(const char first,
                                                         const char second) {
            return '/' == first && ('/' == second || '*' == second);
        }

        const static inline bool is_double_slash_comment(const char first,
                                                         const char second) {
            return '/' == first && '/' == second;
        }
    };
}  // namespace Z::Zaban

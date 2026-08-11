#pragma once

#include <Z/Zaban/Lex/CharUtil.hpp>
#include <cstddef>
#include <cstdint>

namespace Z::Zaban::Lex {
    /**
     * @brief Utility functions for source scanning.
     *
     * Provides helpers for detecting newline sequences and comment markers.
     */
    class ScanUtil {
       public:
        /**
         * @brief Checks whether characters form a newline sequence.
         *
         * Supports `\r\n` and linefeed characters.
         *
         * @param first First character.
         * @param second Second character.
         * @param count Number of consumed characters.
         * @return True if a newline sequence is found.
         */
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

        /**
         * @brief Checks whether characters form a newline sequence.
         *
         * @return True if a newline sequence is found.
         */
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

        /**
         * @brief Checks whether a slash comment starts.
         *
         * Matches `//` and `/ *`.
         *
         * @return True if a comment starts.
         */
        const static inline bool is_either_slash_comment(const char first,
                                                         const char second) {
            return '/' == first && ('/' == second || '*' == second);
        }

        /**
         * @brief Checks for a line comment start (`//`).
         */
        const static inline bool is_double_slash_comment(const char first,
                                                         const char second) {
            return '/' == first && '/' == second;
        }

        /**
         * @brief Checks for a block comment start (`/ *`).
         */
        const static inline bool is_block_slash_comment_start(
            const char first, const char second) {
            return '/' == first && '*' == second;
        }

        /**
         * @brief Checks for a block comment end (`* /`).
         */
        const static inline bool is_block_slash_comment_end(const char first,
                                                            const char second) {
            return '/' == second && '*' == first;
        }
    };
}  // namespace Z::Zaban::Lex

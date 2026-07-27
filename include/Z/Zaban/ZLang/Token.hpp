#pragma once

#include <Z/Zaban/SourcePosition.hpp>
#include <Z/Zaban/ZLang/TokenKind.hpp>
#include <cstdlib>
#include <string>

namespace Z::Zaban::ZLang {
    /** @brief Represents a lexical token produced by the lexer.
     *
     * A token consists of its classification (`TokenKind`) and the
     * corresponding source range identifying where it appears in the
     * original source file.
     *
     * The source range is defined by two source locations (begin and end),
     * each containing an offset within the source and a reference to the
     * originating file.
     */
    struct Token {
        /// Integer type used for source offsets.
        using offset_type = std::size_t;

        /// Type used to reference the source file.
        using file_ref_type = std::string;

        /// The kind of token (identifier, keyword, literal, etc.).
        TokenKind kind;

        /// The source range occupied by the token.
        SourceRange<offset_type, file_ref_type> range;

       public:
        /** @brief Constructs a token from its beginning and ending locations.
         *
         * @param kind The token classification.
         * @param begin The starting location of the token.
         * @param end The ending location of the token.
         */
        Token(TokenKind kind, SourceLocation<offset_type, file_ref_type> begin,
              SourceLocation<offset_type, file_ref_type> end) :
            kind(kind), range(std::move(begin), std::move(end)) {
        }

        /** @brief Constructs a token from an existing source range.
         *
         * @param kind The token classification.
         * @param range The source range occupied by the token.
         */
        Token(TokenKind kind, SourceRange<offset_type, file_ref_type> range) :
            kind(kind), range(std::move(range)) {
        }
    };
}  // namespace Z::Zaban::ZLang

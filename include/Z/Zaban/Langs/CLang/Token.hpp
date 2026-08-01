#pragma once

#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <cstdlib>
#include <string>

namespace Z::Zaban::Langs::CLang {
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
    template<typename T>
    struct Token {
        /// Integer type used for source offsets.
        using offset_type = T;

        /// The kind of token (identifier, keyword, literal, etc.).
        TokenKind kind;

        /// The source range occupied by the token.
        SourcePositionRange<offset_type> range;

       public:
        /** @brief Constructs a token from its beginning and ending locations.
         *
         * @param kind The token classification.
         * @param range The source range occupied by the token.
         */
        Token(TokenKind kind, SourcePositionRange<offset_type> range) :
            kind(kind), range(std::move(range)) {
        }
    };
}  // namespace Z::Zaban::Langs::CLang

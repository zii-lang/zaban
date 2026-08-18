#pragma once

#include <Z/Zaban/SourcePosition.hpp>
#include <cstdlib>
#include <string>

namespace Z::Zaban::Lex {
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
    template<typename TokenKind, typename OffsetType>
    struct Token {
        /// The kind of token (identifier, keyword, literal, etc.).
        TokenKind kind;

        /// The source range occupied by the token.
        OffsetRange<OffsetType> range;

        /// Only meaningful on StringOpen/CharOpen fragments: true when the
        /// fragment ended on a lone backslash at the chunk boundary, so the
        /// first byte of the next chunk is escaped and cannot close the
        /// literal. Ignored for every other kind.
        bool dangling_escape = false;

        /// True when this Numeric fragment ended at a chunk boundary with its
        /// last byte being an exponent prefix (e, E, p, P). A following +/- is
        /// then part of the literal, not a binary operator.
        bool exponent_pending = false;

       public:
        /** @brief Constructs a token from its beginning and ending locations.
         *
         * @param kind The token classification.
         * @param range The source range occupied by the token.
         */
        Token(TokenKind kind, OffsetRange<OffsetType> range) :
            kind(kind), range(std::move(range)) {
        }
    };
}  // namespace Z::Zaban::Lex

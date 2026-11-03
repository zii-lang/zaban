#pragma once

#include <Z/Zaban/Langs/ZLang/LexerTypes.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Token.hpp>

namespace Z::Zaban::Langs::ZLang {
    using Token = Lex::Token<TokenKind, ZLexerPositionType>;

    struct ZTokenFactory {
       public:
        static Token gen(TokenKind kind, ZLexerPositionType begin,
                         ZLexerPositionType end) {
            return Token(kind, OffsetRange<ZLexerPositionType>(begin, end));
        }
    };
}  // namespace Z::Zaban::Langs::ZLang

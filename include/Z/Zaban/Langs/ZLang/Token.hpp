#pragma once

#include <Z/Zaban/Langs/ZLang/LexerTypes.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Token.hpp>

namespace Z::Zaban::Langs::ZLang {
    using Token = Lex::Token<TokenKind, ZLexerPositionType>;
}  // namespace Z::Zaban::Langs::ZLang

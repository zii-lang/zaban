#pragma once

#include <Z/Zaban/Langs/CLang/LexerTypes.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Token.hpp>

namespace Z::Zaban::Langs::CLang {
    using Token = Lex::Token<TokenKind, CLexerPositionType>;
}  // namespace Z::Zaban::Langs::CLang

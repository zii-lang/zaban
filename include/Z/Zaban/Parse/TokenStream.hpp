#pragma once

#include <Z/Zaban/Lex/Token.hpp>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class TokenStream {
       private:
       public:
        Lex::Token<TokenKind, OffsetType> peek();
        void                              advance();
        bool match(Lex::Token<TokenKind, OffsetType>);
    };
}  // namespace Z::Zaban::Parse

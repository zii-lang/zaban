#pragma once

#include <Z/Zaban/Lex/Token.hpp>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class TokenStream {
       private:
       public:
        Token<TokenKind, OffsetType> peek();
        void                         advance();
        bool                         match(Token<TokenKind, OffsetType>);
    };
}  // namespace Z::Zaban::Parse

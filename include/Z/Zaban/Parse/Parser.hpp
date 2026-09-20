#pragma once

#include <Z/Zaban/Parse/TokenStream.hpp>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class Parser {
       public:
        virtual ~Parser() = default;

        // TODO: parse should return AST.
        virtual void parse(TokenStream<TokenKind, OffsetType> stream) = 0;
    };
}  // namespace Z::Zaban::Parse

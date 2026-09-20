#pragma once

#include <Z/Zaban/AST/Module.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class Parser {
       public:
        virtual ~Parser() = default;

        virtual AST::Module parse(
            TokenStream<TokenKind, OffsetType> stream) = 0;
    };
}  // namespace Z::Zaban::Parse

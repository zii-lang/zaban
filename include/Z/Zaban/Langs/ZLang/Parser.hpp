#pragma once

#include <Z/Zaban/AST/Module.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Parse/Parser.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>

namespace Z::Zaban::Langs::ZLang {
    class ZParser : public Parse::Parser<ZLang::TokenKind, std::size_t> {
       public:
        AST::Module<std::size_t> parse(
            Parse::TokenStream<ZLang::TokenKind, std::size_t> stream);
    };
}  // namespace Z::Zaban::Langs::ZLang

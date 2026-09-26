#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <vector>

namespace Z::Zaban::Langs::ZLang {
    AST::Module<std::size_t> Z::Zaban::Langs::ZLang::ZParser::parse(
        Parse::TokenStream<ZLang::TokenKind, std::size_t> stream) {
        AST::Module<> module{};

        return module;
    }
}  // namespace Z::Zaban::Langs::ZLang

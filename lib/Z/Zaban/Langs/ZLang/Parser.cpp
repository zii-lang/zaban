#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>
#include <memory>
#include <vector>

namespace Z::Zaban::Langs::ZLang {
    ZParser::ZParser(ZTokenStream stream) : m_stream(stream) {};
    ZParser::ZParser(ZTokenStream&& stream) : m_stream(std::move(stream)) {};

    AST::Module<ZOffsetType> Z::Zaban::Langs::ZLang::ZParser::parse() {
        AST::Module<ZOffsetType> module{};

        while (!this->m_stream.end()) {
            const AST::Statement<> statement = parse_statement();
            if (statement != nullptr) {
            }
        }

        return module;
    }

}  // namespace Z::Zaban::Langs::ZLang

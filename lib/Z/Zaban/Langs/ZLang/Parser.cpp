#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>
#include <memory>
#include <vector>

namespace Z::Zaban::Langs::ZLang {
    ZParser::ZParser(ZTokenStream stream) : m_stream(stream) {};
    ZParser::ZParser(ZTokenStream&& stream) : m_stream(std::move(stream)) {};

    AST::Declaration<ZOffsetType> ZParser::parse_declaration() {
        auto token = this->m_stream.peek();
        switch (token->kind) {
            case ZTokenKind::Let:
                this->m_stream.advance();
                return parse_let_declaration();
            case ZTokenKind::Type:
                this->m_stream.advance();
                return parse_type_declaration();
            default:
                break;
        }
        return nullptr;
    }

    AST::Statement<ZOffsetType> ZParser::parse_statement() {
        auto token = this->m_stream.peek();
        switch (token->kind) {
            case ZTokenKind::Type:
            case ZTokenKind::Let: {
                auto decl = parse_declaration();
                //
            } break;
            default:
        }
        return nullptr;
    }

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

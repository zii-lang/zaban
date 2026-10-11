#include <Z/Zaban/AST/Statements/DeclarationStatement.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Statement<ZOffsetType> ZParser::parse_statement() const {
        auto token = this->m_stream.peek();
        switch (token->kind) {
            case ZTokenKind::Type:
            case ZTokenKind::Let: {
                auto declaration = parse_declaration();
                auto token       = this->m_stream.peek();
                if (!token || token->kind != ZTokenKind::Semicolon) {
                    // TODO: add error, we require semicolon after declaration
                    // statement.
                    return nullptr;
                }
                this->m_stream.advance();
                AST::Statements::DeclarationStatement<ZOffsetType>
                    declaration_statement(declaration);
                declaration_statement.set_location(OffsetRange<ZOffsetType>(
                    declaration->location().begin, token->range.end));
                return declaration_statement.as_ptr();
            } break;
            default:
                break;
        }
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang

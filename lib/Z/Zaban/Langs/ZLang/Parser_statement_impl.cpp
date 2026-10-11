#include <Z/Zaban/AST/Statements/BlockStatement.hpp>
#include <Z/Zaban/AST/Statements/DeclarationStatement.hpp>
#include <Z/Zaban/AST/Statements/ExpressionStatement.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Statement<ZOffsetType> ZParser::parse_block_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::LBrace) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_position = token->range.begin;

        std::vector<AST::Statement<ZOffsetType>> block_body;
        while (!this->m_stream.end() &&
               this->m_stream.peek()->kind != ZTokenKind::RBrace) {
            auto statement = this->parse_statement();
            if (!statement) {
                return nullptr;
            }
            block_body.emplace_back(std::move(statement));
        }

        token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::RBrace) {
            // TODO: report error required enclosing '}'
            return nullptr;
        }
        this->m_stream.advance();  // consume '}'
        AST::Statements::BlockStatement<ZOffsetType> block(block_body);
        block.set_location(
            OffsetRange<ZOffsetType>(start_position, token->range.end));
        return block.as_ptr();
    }

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
            case ZTokenKind::LBrace:
                return this->parse_block_statement();
            case ZTokenKind::Label:
                return this->parse_label_statement();
            case ZTokenKind::Break:
                return this->parse_break_statement();
            case ZTokenKind::Continue:
                return this->parse_continue_statement();
            case ZTokenKind::Goto:
                return this->parse_goto_statement();
            case ZTokenKind::Return:
                return this->parse_return_statement();
            default:
                AST::Expression<ZOffsetType> expression =
                    this->parse_expression();
                if (!expression) {
                    // TODO: report error;
                    return nullptr;
                }
                auto token = this->m_stream.peek();
                if (!token || token->kind != ZTokenKind::Semicolon) {
                    // TODO: report error required semicolon after expression
                    // statement.
                    return nullptr;
                }
                this->m_stream.advance();  // consume ';'
                AST::Statements::ExpressionStatement<ZOffsetType>
                    expression_statement(std::move(expression));
                expression_statement.set_location(OffsetRange<ZOffsetType>(
                    expression->location().begin, token->range.end));
                return expression_statement.as_ptr();
        }
    }
}  // namespace Z::Zaban::Langs::ZLang

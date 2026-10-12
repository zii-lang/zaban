#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Statements/BlockStatement.hpp>
#include <Z/Zaban/AST/Statements/DeclarationStatement.hpp>
#include <Z/Zaban/AST/Statements/ExpressionStatement.hpp>
#include <Z/Zaban/AST/Statements/FlowStatement.hpp>
#include <Z/Zaban/AST/Statements/LabelStatement.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Statement<ZOffsetType> ZParser::parse_return_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Return) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_location = token->range.begin;
        this->m_stream.advance();  // consume 'return'

        if (this->m_stream.match(ZTokenKind::Semicolon)) {
            AST::Statements::FlowStatement<ZOffsetType> return_statement(
                AST::Statements::FlowKind::Return);
            return_statement.set_location(OffsetRange<ZOffsetType>(
                start_location, this->m_stream.previous()->range.end));
            return return_statement.as_ptr();
        }
        auto expression = this->parse_expression();
        token           = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Semicolon) {
            // TODO: required semicolon and got unexpected token.
            return nullptr;
        }
        AST::Statements::FlowStatement<ZOffsetType> return_statement(
            AST::Statements::FlowKind::Return, expression);
        return_statement.set_location(
            OffsetRange<ZOffsetType>(start_location, token->range.end));
        return return_statement.as_ptr();
    }

    AST::Statement<ZOffsetType> ZParser::parse_goto_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Goto) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_location = token->range.begin;
        this->m_stream.advance();  // consume 'goto'

        AST::Atomic<ZOffsetType> identifier = this->parse_identifier_atomic();
        if (!identifier) {
            // TODO: goto requires identifier afterwards.
            return nullptr;
        }

        token = this->m_stream.peek();
        if (!this->m_stream.match(ZTokenKind::Semicolon)) {
            // TODO: error missing semicolon after break statement.
            return nullptr;
        }
        this->m_stream.advance();  // consume ';'

        AST::Statements::FlowStatement<ZOffsetType> goto_statement(
            AST::Statements::FlowKind::Goto, identifier);

        goto_statement.set_location(
            OffsetRange<ZOffsetType>(start_location, token->range.end));

        return goto_statement.as_ptr();
    }

    AST::Statement<ZOffsetType> ZParser::parse_continue_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Continue) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_location = token->range.begin;
        this->m_stream.advance();  // consume 'continue'

        token = this->m_stream.peek();
        if (!token) {
            // TODO: unexpected eof.
            return nullptr;
        }

        AST::Atomic<ZOffsetType> identifier = nullptr;
        if (token->kind == ZTokenKind::Identifier) {
            identifier = this->parse_identifier_atomic();
        }

        token = this->m_stream.peek();
        if (!this->m_stream.match(ZTokenKind::Semicolon)) {
            // TODO: error missing semicolon after break statement.
            return nullptr;
        }
        this->m_stream.advance();  // consume ';'

        AST::Statements::FlowStatement<ZOffsetType> continue_statement(
            AST::Statements::FlowKind::Continue, identifier);

        continue_statement.set_location(
            OffsetRange<ZOffsetType>(start_location, token->range.end));

        return continue_statement.as_ptr();
    }

    AST::Statement<ZOffsetType> ZParser::parse_break_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Break) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_location = token->range.begin;
        this->m_stream.advance();  // consume 'break'

        token = this->m_stream.peek();
        if (!token) {
            // TODO: unexpected eof.
            return nullptr;
        }

        AST::Atomic<ZOffsetType> identifier = nullptr;
        if (token->kind == ZTokenKind::Identifier) {
            identifier = this->parse_identifier_atomic();
        }

        token = this->m_stream.peek();
        if (!this->m_stream.match(ZTokenKind::Semicolon)) {
            // TODO: error missing semicolon after break statement.
            return nullptr;
        }
        this->m_stream.advance();  // consume ';'

        AST::Statements::FlowStatement<ZOffsetType> break_statement(
            AST::Statements::FlowKind::Break, identifier);

        break_statement.set_location(
            OffsetRange<ZOffsetType>(start_location, token->range.end));

        return break_statement.as_ptr();
    }

    AST::Statement<ZOffsetType> ZParser::parse_label_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Label) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_location = token->range.begin;

        this->m_stream.advance();  // consume 'label'
        auto identifier = this->parse_identifier_atomic();
        if (!identifier) {
            // TODO: report error label requires identifier afterwards.
            return nullptr;
        }
        auto end_location = identifier->location().end;

        if (!this->m_stream.match(ZTokenKind::Colon)) {
            // TODO: report error if strict.
        } else {
            end_location = this->m_stream.previous()->range.end;
        }

        AST::Statements::LabelStatement<ZOffsetType> label_statement(
            identifier);
        label_statement.set_location(
            OffsetRange<ZOffsetType>(start_location, end_location));
        return label_statement.as_ptr();
    }

    AST::Statement<ZOffsetType> ZParser::parse_block_statement() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::LBrace) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_position = token->range.begin;
        this->m_stream.advance();

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

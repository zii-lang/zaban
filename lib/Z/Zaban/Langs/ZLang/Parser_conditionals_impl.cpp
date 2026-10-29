#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

// Internal
#include <Z/Zaban/Langs/ZLang/AST/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expressions/IfExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/HeaderConditions.hpp>
#include <Z/Zaban/Langs/ZLang/AST/WhenClause.hpp>

namespace Z::Zaban::Langs::ZLang {
    Zaban::AST::Expression<ZOffsetType> ZParser::parse_if() const {
        auto token = this->m_stream.peek();
        if (!token && token->kind != TokenKind::If) ZABAN_UNLIKELY {
                // TODO: report error this can't happen
                return nullptr;
            }
        auto begin_location = token->range.begin;
        this->m_stream.advance();

        token = this->m_stream.peek();

        if (!token) {
            // TODO: report error unexpected eof.
            return nullptr;
        }

        AST::ZLang::HeaderConditions<ZOffsetType> headers     = nullptr;
        AST::ZLang::ElseClause<ZOffsetType>       else_clause = nullptr;
        // probably short hand or condition header.
        if (token->kind != TokenKind::When) {
            auto expression = this->parse_expression();
            if (!expression) {
                // TODO: report we need expression here.
                return nullptr;
            }
            token = this->m_stream.peek();

            if (token->kind == TokenKind::EqualBig) {
                this->m_stream.advance();
                auto statement = this->parse_statement();
                if (!statement) {
                    // TODO: report error.
                    return nullptr;
                }
                token = this->m_stream.peek();
                if (!token) {
                    // TODO: report error.
                    return nullptr;
                }
                if (token->kind == TokenKind::Else) {
                    else_clause = this->parse_else_clause();
                    if (!else_clause) {
                        return nullptr;
                    }
                }
                token = this->m_stream.peek();
                if (!m_stream.match(TokenKind::EndIf)) {
                    // TODO: report error.
                    return nullptr;
                }
                auto if_node =
                    AST::ZLang::Expressions::IfExpressionNode<ZOffsetType>(
                        expression, else_clause);
                if_node.set_location(
                    OffsetRange<ZOffsetType>(begin_location, token->range.end));
                return if_node.as_ptr();
            } else if (token->kind == TokenKind::Comma) {
                std::vector<AST::Expression<ZOffsetType>> v_header{};
                v_header.emplace_back(std::move(expression));
                this->m_stream.advance();
                while (true) {
                    auto header_expr = this->parse_expression();
                    if (!header_expr) {
                        // TODO: report error.
                        return nullptr;
                    }
                    v_header.emplace_back(std::move(header_expr));
                    if (!this->m_stream.match(TokenKind::Comma)) {
                        break;
                    }
                }
                AST::ZLang::HeaderConditionsNode<ZOffsetType> headers_node(
                    std::move(v_header));
                headers = headers_node.as_ptr();
            } else if (token->kind == TokenKind::When) {
            }
        }

        token = this->m_stream.peek();
        if (token == nullptr) {
            // TODO: report error.
            return nullptr;
        }
        std::vector<AST::ZLang::WhenClause<ZOffsetType>> whens{};
        while (token->kind == TokenKind::When) {
            auto when_clause = this->parse_when_clause();
            if (!when_clause) {
                // TODO: report an error.
                return nullptr;
            }
            whens.emplace_back(when_clause);
        }

        token = this->m_stream.peek();
        if (token->kind == TokenKind::Else) {
            else_clause = this->parse_else_clause();
        }

        token = this->m_stream.peek();
        if (!token && token->kind != TokenKind::EndIf) {
            // TODO: report unclosed end sentinal.
            return nullptr;
        }

        AST::ZLang::Expressions::IfExpressionNode<ZOffsetType> node(
            whens, else_clause);
        node.set_location(
            OffsetRange<ZOffsetType>(begin_location, token->range.end));
        return node.as_ptr();
    }

    Zaban::AST::Expression<ZOffsetType> ZParser::parse_loop() const {
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang

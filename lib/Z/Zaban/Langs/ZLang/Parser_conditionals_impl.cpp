#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

// Internal
#include <Z/Zaban/AST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expressions/IfExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/HeaderConditions.hpp>
#include <Z/Zaban/Langs/ZLang/AST/WhenClause.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::ZLang::Condition<ZOffsetType> ZParser::parse_half_condition() const {
        auto token = this->m_stream.peek();
        if (!token) ZABAN_UNLIKELY {
                // report error, we already checked this.
                return nullptr;
            }

        AST::BooleanOperator op = AST::BooleanOperator::Eq;

        switch (token->kind) {
            case TokenKind::EqualEqual:
                op = AST::BooleanOperator::Eq;
                break;
            case TokenKind::ExclamEqual:
                op = AST::BooleanOperator::Neq;
                break;
            case TokenKind::Lesser:
                op = AST::BooleanOperator::Lt;
                break;
            case TokenKind::LesserEqual:
                op = AST::BooleanOperator::Lte;
                break;
            case TokenKind::Greater:
                op = AST::BooleanOperator::Gt;
                break;
            case TokenKind::GreaterEqual:
                op = AST::BooleanOperator::Gte;
                break;
            default:
                // TODO: report error unknown half condition operator.
                return nullptr;
        }

        auto expr = this->parse_expression();
        if (!expr) {
            // TODO: report error need expression after operator of
            // half-condition.
            return nullptr;
        }

        AST::ZLang::HalfCondition<ZOffsetType> half_condition(op, expr);
        half_condition.set_location(
            OffsetRange<ZOffsetType>(token->range.begin, expr->location().end));

        return half_condition.as_ptr();
    }

    AST::ZLang::Condition<ZOffsetType> ZParser::parse_condition() const {
        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error.
            return nullptr;
        }

        AST::ZLang::Condition<ZOffsetType> lhs_condition = nullptr;
        switch (token->kind) {
            case TokenKind::EqualEqual:
            case TokenKind::ExclamEqual:
            case TokenKind::Lesser:
            case TokenKind::LesserEqual:
            case TokenKind::Greater:
            case TokenKind::GreaterEqual:
                lhs_condition = this->parse_half_condition();
                if (!lhs_condition) {
                    return nullptr;
                }
                break;
            case TokenKind::LBrak:
                // PassAll or Indexed
                break;
            case TokenKind::LParen:
				// Group
                break;
            default:
                auto expr = this->parse_expression();
        }
        return nullptr;
    }

    AST::ZLang::ElseClause<ZOffsetType> ZParser::parse_else_clause() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != TokenKind::Else) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto begin_location = token->range.begin;
        this->m_stream.advance();

        if (!this->m_stream.match(TokenKind::EqualBig)) {
            // TODO: report require => after else.
            return nullptr;
        }

        auto statement = this->parse_statement();
        if (!statement) {
            // TODO: report required statement after else.
            return nullptr;
        }

        AST::ZLang::ElseNode<ZOffsetType> else_node(std::move(statement));
        else_node.set_location(OffsetRange<ZOffsetType>(
            begin_location, statement->location().end));

        return else_node.as_ptr();
    }

    AST::ZLang::WhenClause<ZOffsetType> ZParser::parse_when_clause() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != TokenKind::When) ZABAN_UNLIKELY {
                return nullptr;
            }

        auto begin_location = token->range.begin;
        this->m_stream.advance();

        bool has_all = false;

        if (this->m_stream.match(TokenKind::All)) {
            has_all = true;
        }

        auto condition = this->parse_condition();
        if (!condition) {
            // TODO: report error required condition.
            return nullptr;
        }

        if (!this->m_stream.match(TokenKind::EqualBig)) {
            // TODO: required => after condition.
            return nullptr;
        }

        auto statement = this->parse_statement();
        if (!statement) {
            // TODO: report no statement error.
            return nullptr;
        }

        AST::ZLang::WhenNode<ZOffsetType> when_node(has_all, condition,
                                                    statement);
        when_node.set_location(OffsetRange<ZOffsetType>(
            begin_location, statement->location().end));

        return when_node.as_ptr();
    }

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

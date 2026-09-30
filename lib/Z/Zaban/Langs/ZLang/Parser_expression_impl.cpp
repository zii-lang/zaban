#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Expressions/AssignmentExpression.hpp>
#include <Z/Zaban/AST/Expressions/GroupExpression.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Expression<ZOffsetType> ZParser::parse_expression() const {
        auto expr = this->parse_assignment();
        if (expr == nullptr) {
            // TODO: report error.
        }
        return expr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_assignment() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_logical_or() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_logical_and() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_or() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_xor() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_and() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_equality() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_comparison() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_shifting() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_additive() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_multipicative() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_unary() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_suffix() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_primary() const {
        auto token = this->m_stream.peek();

        if (token->kind == ZLexerTokenKind::LParen) {
            return this->parse_group();
        }
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_group() const {
        auto token = this->m_stream.peek();

        if (token == nullptr) ZABAN_UNLIKELY {
                // TODO: report error.
                return nullptr;
            }

        if (token->kind != ZLexerTokenKind::LParen) {
            // TODO: report error group needs to start with '('.
            return nullptr;
        }

        auto start_token = std::move(token);
        this->m_stream.advance();
        auto inner = this->parse_expression();
        if (inner == nullptr) {
            // TODO: report unable to parse group inner expression.
            return nullptr;
        }
        token = this->m_stream.peek();
        if (token == nullptr) ZABAN_UNLIKELY {
                // TODO: report unexpexted end.
                return nullptr;
            }

        if (token->kind != ZLexerTokenKind::RParen) {
            // TODO: report required right parenthesis.
            return nullptr;
        }

        this->m_stream.advance();

        auto group = AST::Expressions::GroupExpression<ZOffsetType>(
            std::move(inner), OffsetRange<ZOffsetType>(start_token->range.begin,
                                                       token->range.end));

        return group.get_ptr();
    }

}  // namespace Z::Zaban::Langs::ZLang

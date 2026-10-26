#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

#include "Z/Zaban/Langs/ZLang/AST/Expressions/IfExpression.hpp"
#include "Z/Zaban/Langs/ZLang/AST/Extension/ElseClause.hpp"

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

        // probably short hand or condition header.
        if (token->kind != TokenKind::When) {
            auto expression = this->parse_expression();
            if (!expression) {
                // TODO: report we need expression here.
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
                AST::Extension::ElseClause<ZOffsetType> else_clause = nullptr;
                if (token->kind == TokenKind::Else) {
                    // TODO: parse else
                }
                token = this->m_stream.peek();
                if (!m_stream.match(TokenKind::EndIf)) {
                    // TODO: report error.
                    return nullptr;
                }
                auto if_node = AST::Expressions::IfExpressionNode<ZOffsetType>(
                    expression, else_clause);
                if_node.set_location(
                    OffsetRange<ZOffsetType>(begin_location, token->range.end));
                return if_node.as_ptr();
            } else if (token->kind == TokenKind::Comma) {
                // Condition header
            } else if (token->kind == TokenKind::When) {
            }
        }
        return nullptr;
    }

    Zaban::AST::Expression<ZOffsetType> ZParser::parse_loop() const {
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang

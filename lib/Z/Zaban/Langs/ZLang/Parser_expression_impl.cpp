#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Expressions/AssignmentExpression.hpp>
#include <Z/Zaban/AST/Expressions/BinaryExpression.hpp>
#include <Z/Zaban/AST/Expressions/GroupExpression.hpp>
#include <Z/Zaban/AST/Expressions/PrefixExpression.hpp>
#include <Z/Zaban/AST/Expressions/PrimaryExpression.hpp>
#include <Z/Zaban/AST/Expressions/SuffixExpression.hpp>
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
        using AssignmentOperator = AST::Expressions::AssignmentOperator;

        auto               left          = this->parse_logical_or();
        bool               is_assignment = false;
        AssignmentOperator op            = AssignmentOperator::None;

        auto token = this->m_stream.peek();
        switch (token->kind) {
            case ZTokenKind::Equal:
                is_assignment = true;
                break;
            case ZTokenKind::PlusEqual:
                is_assignment = true;
                op            = AssignmentOperator::Add;
                break;
            case ZTokenKind::MinusEqual:
                is_assignment = true;
                op            = AssignmentOperator::Sub;
                break;
            case ZTokenKind::AsteriskEqual:
                is_assignment = true;
                op            = AssignmentOperator::Mul;
                break;
            case ZTokenKind::SlashEqual:
                is_assignment = true;
                op            = AssignmentOperator::Div;
                break;
            case ZTokenKind::PercentEqual:
                is_assignment = true;
                op            = AssignmentOperator::Mod;
                break;
            case ZTokenKind::AmpEqual:
                is_assignment = true;
                op            = AssignmentOperator::And;
                break;
            case ZTokenKind::PipeEqual:
                is_assignment = true;
                op            = AssignmentOperator::Or;
                break;
            case ZTokenKind::LesserLesserEqual:
                is_assignment = true;
                op            = AssignmentOperator::Shl;
                break;
            case ZTokenKind::GreaterGreaterEqual:
                is_assignment = true;
                op            = AssignmentOperator::Shr;
                break;
            // TODO: add xor?
            default:
                break;
        }
        if (is_assignment) {
            this->m_stream.advance();  // consume assignment operator.

            AST::Expression right = this->parse_assignment();
            if (!right) {
                auto annotation = this->parse_annotation();
                if (!annotation) {
                    // TODO: report error we expect a right value after
                    // assignment.
                    return nullptr;
                }

                AST::Expressions::AssignmentExpression<ZOffsetType> expression(
                    op, left, annotation);
                expression.set_location(OffsetRange<ZOffsetType>(
                    left->location().begin, annotation->location().end));
                return expression.as_ptr();
            }

            AST::Expressions::AssignmentExpression<ZOffsetType> expression(
                op, left, right);

            expression.set_location(OffsetRange<ZOffsetType>(
                left->location().begin, right->location().end));
            return expression.as_ptr();
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_logical_or() const {
        auto left = this->parse_logical_and();

        if (!left) {
            return nullptr;
        }

        if (this->m_stream.match(ZTokenKind::PipePipe)) {
            auto right = this->parse_logical_or();
            if (!right) {
                // TODO: report error.
                return nullptr;
            }

            AST::Expressions::BinaryExpression<ZOffsetType> expr(
                left, right, AST::Expressions::BinaryOperator::Lor);

            expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                       right->location().end));
            return expr.as_ptr();
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_logical_and() const {
        auto left = this->parse_bitwise_or();

        if (!left) {
            return nullptr;
        }

        if (this->m_stream.match(ZTokenKind::AmpAmp)) {
            auto right = this->parse_logical_and();
            if (!right) {
                // TODO: report error.
                return nullptr;
            }

            AST::Expressions::BinaryExpression<ZOffsetType> expr(
                left, right, AST::Expressions::BinaryOperator::Land);

            expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                       right->location().end));
            return expr.as_ptr();
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_or() const {
        auto left = this->parse_bitwise_xor();

        if (!left) {
            return nullptr;
        }

        if (this->m_stream.match(ZTokenKind::Pipe)) {
            auto right = this->parse_bitwise_or();
            if (!right) {
                // TODO: report error.
                return nullptr;
            }

            AST::Expressions::BinaryExpression<ZOffsetType> expr(
                left, right, AST::Expressions::BinaryOperator::Or);

            expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                       right->location().end));
            return expr.as_ptr();
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_xor() const {
        auto left = this->parse_bitwise_and();

        if (!left) {
            return nullptr;
        }

        if (this->m_stream.match(ZTokenKind::Caret)) {
            auto right = this->parse_bitwise_xor();
            if (!right) {
                // TODO: report error.
                return nullptr;
            }

            AST::Expressions::BinaryExpression<ZOffsetType> expr(
                left, right, AST::Expressions::BinaryOperator::Xor);

            expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                       right->location().end));
            return expr.as_ptr();
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_and() const {
        auto left = this->parse_equality();

        if (!left) {
            return nullptr;
        }

        if (this->m_stream.match(ZTokenKind::Amp)) {
            auto right = this->parse_bitwise_and();
            if (!right) {
                // TODO: report error.
                return nullptr;
            }

            AST::Expressions::BinaryExpression<ZOffsetType> expr(
                left, right, AST::Expressions::BinaryOperator::And);

            expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                       right->location().end));
            return expr.as_ptr();
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_equality() const {
        auto left = this->parse_comparison();

        if (!left) {
            return nullptr;
        }

        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expression<ZOffsetType>     right = nullptr;
        AST::Expressions::BinaryOperator op;
        if (token->kind == TokenKind::EqualEqual) {
            this->m_stream.advance();
            op = AST::Expressions::BinaryOperator::Eq;
        } else if (token->kind == TokenKind::ExclamEqual) {
            this->m_stream.advance();
            op = AST::Expressions::BinaryOperator::Neq;
        } else {
            return left;
        }

        right = this->parse_equality();
        if (!right) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expressions::BinaryExpression<ZOffsetType> expr(left, right, op);
        expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                   right->location().end));
        return expr.as_ptr();
    }

    AST::Expression<ZOffsetType> ZParser::parse_comparison() const {
        auto left = this->parse_shifting();

        if (!left) {
            return nullptr;
        }

        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expression<ZOffsetType>     right = nullptr;
        AST::Expressions::BinaryOperator op;

        switch (token->kind) {
            case ZTokenKind::Lesser:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Lt;
                break;
            case ZTokenKind::LesserEqual:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Lte;
                break;
            case ZTokenKind::Greater:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Gt;
                break;
            case ZTokenKind::GreaterEqual:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Gte;
                break;
            default:
                return left;
        }

        right = this->parse_comparison();
        if (!right) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expressions::BinaryExpression<ZOffsetType> expr(left, right, op);
        expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                   right->location().end));
        return expr.as_ptr();
    }

    AST::Expression<ZOffsetType> ZParser::parse_shifting() const {
        auto left = this->parse_additive();

        if (!left) {
            return nullptr;
        }

        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expression<ZOffsetType>     right = nullptr;
        AST::Expressions::BinaryOperator op;

        switch (token->kind) {
            case ZTokenKind::LesserLesser:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Shl;
                break;
            case ZTokenKind::GreaterGreater:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Shr;
                break;
            default:
                return left;
        }

        right = this->parse_shifting();
        if (!right) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expressions::BinaryExpression<ZOffsetType> expr(left, right, op);
        expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                   right->location().end));
        return expr.as_ptr();
    }

    AST::Expression<ZOffsetType> ZParser::parse_additive() const {
        auto left = this->parse_multipicative();

        if (!left) {
            return nullptr;
        }

        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expression<ZOffsetType>     right = nullptr;
        AST::Expressions::BinaryOperator op;

        switch (token->kind) {
            case ZTokenKind::Plus:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Add;
                break;
            case ZTokenKind::Minus:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Sub;
                break;
            default:
                return left;
        }

        right = this->parse_additive();
        if (!right) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expressions::BinaryExpression<ZOffsetType> expr(left, right, op);
        expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                   right->location().end));
        return expr.as_ptr();
    }

    AST::Expression<ZOffsetType> ZParser::parse_multipicative() const {
        auto left = this->parse_unary();

        if (!left) {
            return nullptr;
        }

        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expression<ZOffsetType>     right = nullptr;
        AST::Expressions::BinaryOperator op;

        switch (token->kind) {
            case ZTokenKind::Asterisk:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Mul;
                break;
            case ZTokenKind::Slash:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Div;
                break;
            case ZTokenKind::Percent:
                this->m_stream.advance();
                op = AST::Expressions::BinaryOperator::Mod;
                break;
            default:
                return left;
        }

        right = this->parse_multipicative();
        if (!right) {
            // TODO: report error.
            return nullptr;
        }

        AST::Expressions::BinaryExpression<ZOffsetType> expr(left, right, op);
        expr.set_location(OffsetRange<ZOffsetType>(left->location().begin,
                                                   right->location().end));
        return expr.as_ptr();
    }

    AST::Expression<ZOffsetType> ZParser::parse_unary() const {
        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report error eof.
            return nullptr;
        }
        auto start_position = token->range.begin;

        AST::Expressions::PrefixOperator op;
        switch (token->kind) {
            case ZTokenKind::Minus:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::Neg;
                break;
            case ZTokenKind::Exclam:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::LNeg;
                break;
            case ZTokenKind::Tilde:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::BNeg;
                break;
            case ZTokenKind::PlusPlus:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::AddAdd;
                break;
            case ZTokenKind::MinusMinus:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::SubSub;
                break;
            case ZTokenKind::AmpOp:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::AddrOf;
                break;
            case ZTokenKind::AsteriskOp:
                this->m_stream.advance();
                op = AST::Expressions::PrefixOperator::Deref;
                break;
            default:
                return this->parse_suffix();
        }

        AST::Expression operand = this->parse_unary();
        if (!operand) {
            // TODO: report
            return nullptr;
        }
        AST::Expressions::PrefixExpression<ZOffsetType> expr(operand, op);
        expr.set_location(
            OffsetRange<ZOffsetType>(start_position, operand->location().end));
        return expr.as_ptr();
    }

    AST::Expression<ZOffsetType> ZParser::parse_suffix() const {
        AST::Expression left = this->parse_primary();

        while (true) {
            auto token = this->m_stream.peek();
            if (!token) {
                // TODO: report error eof.
                return nullptr;
            }

            if (token->kind == ZTokenKind::PlusPlus) {
                this->m_stream.advance();
                AST::Expressions::SuffixExpression<ZOffsetType> sfx(
                    left, AST::Expressions::SuffixOperator::AddAdd);
                sfx.set_location(OffsetRange<ZOffsetType>(
                    left->location().begin, token->range.end));
                left = sfx.as_ptr();
                continue;
            }

            if (token->kind == ZTokenKind::MinusMinus) {
                this->m_stream.advance();
                AST::Expressions::SuffixExpression<ZOffsetType> sfx(
                    left, AST::Expressions::SuffixOperator::SubSub);
                sfx.set_location(OffsetRange<ZOffsetType>(
                    left->location().begin, token->range.end));
                left = sfx.as_ptr();
                continue;
            }

            // TODO: add index access, member access and call expressions.

            break;
        }

        return left;
    }

    AST::Expression<ZOffsetType> ZParser::parse_primary() const {
        auto token = this->m_stream.peek();

        if (token == nullptr) ZABAN_UNLIKELY {
                // TODO: report error.
                return nullptr;
            }

        switch (token->kind) {
            case ZTokenKind::True:
            case ZTokenKind::False:
            case ZTokenKind::Null:
            case ZTokenKind::Numeric:
            case TokenKind::String:
            case TokenKind::LBrak:
            case TokenKind::LBrace: {
                auto inner_atomic = this->parse_literal_atomic();
                auto expr = AST::Expressions::PrimaryExpression<ZOffsetType>(
                    inner_atomic);
                return expr.get_ptr();
            }
            case ZTokenKind::Identifier: {
                auto inner_atomic = this->parse_identifier_atomic();
                auto expr = AST::Expressions::PrimaryExpression<ZOffsetType>(
                    inner_atomic);
                return expr.get_ptr();
            }
            case ZTokenKind::LParen:
                return this->parse_group();
            case ZTokenKind::If:
                // TODO: not implemented.
                return nullptr;
            case ZTokenKind::Loop:
                // TODO: not implemented.
                return nullptr;
            case ZTokenKind::Func:
                // TODO: not implemented.
                return nullptr;
            default:
                break;
        }
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_group() const {
        auto token = this->m_stream.peek();

        if (token == nullptr) ZABAN_UNLIKELY {
                // TODO: report error.
                return nullptr;
            }

        if (token->kind != ZTokenKind::LParen) {
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

        if (token->kind != ZTokenKind::RParen) {
            // TODO: report required right parenthesis.
            return nullptr;
        }

        this->m_stream.advance();

        auto group =
            AST::Expressions::GroupExpression<ZOffsetType>(std::move(inner));

        return group.get_ptr();
    }

}  // namespace Z::Zaban::Langs::ZLang

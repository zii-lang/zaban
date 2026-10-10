
#include <Z/Zaban/AST/Expressions/AssignmentExpression.hpp>
#include <Z/Zaban/AST/Expressions/BinaryExpression.hpp>
#include <Z/Zaban/Langs/CLang/AST/Expressions/TernaryExpression.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <memory>
#include <optional>

#include "Z/Zaban/AST/Expressions/PrefixExpression.hpp"
#include "Z/Zaban/AST/Expressions/SuffixExpression.hpp"

namespace {

    using CTokenKind         = Z::Zaban::Langs::CLang::CTokenKind;
    using AssignmentOperator = Z::Zaban::AST::Expressions::AssignmentOperator;
    using BinaryOperator     = Z::Zaban::AST::Expressions::BinaryOperator;
    using PrefixOperator     = Z::Zaban::AST::Expressions::PrefixOperator;
    using SuffixOperator     = Z::Zaban::AST::Expressions::SuffixOperator;

    // maps a postfix token to its operator. any non-suffix token maps to
    // nullopt
    std::optional<SuffixOperator> assign_suffix_op(const CTokenKind kind) {
        switch (kind) {
            case CTokenKind::PlusPlus:
                return SuffixOperator::AddAdd;
            case CTokenKind::MinusMinus:
                return SuffixOperator::SubSub;
            default:
                return std::nullopt;
        }
    }
    // maps a prefixOp token to its operator. any non-prefix token maps to
    // nullopt
    std::optional<PrefixOperator> assign_prefix_op(const CTokenKind kind) {
        switch (kind) {
            case CTokenKind::PlusPlus:
                return PrefixOperator::AddAdd;
            case CTokenKind::MinusMinus:
                return PrefixOperator::SubSub;
            case CTokenKind::Minus:
                return PrefixOperator::Neg;
            case CTokenKind::Exclam:
                return PrefixOperator::LNeg;
            case CTokenKind::Tilde:
                return PrefixOperator::BNeg;
            case CTokenKind::Amp:
                return PrefixOperator::AddrOf;
            case CTokenKind::Asterisk:
                return PrefixOperator::Deref;
            default:
                return std::nullopt;
        }
    }

    // maps a binaryop token to its operator. any non-binaryop token maps to
    // nullopt
    std::optional<BinaryOperator> assign_bin_op(const CTokenKind kind) {
        switch (kind) {
            case CTokenKind::GreaterEqual:
                return BinaryOperator::Gte;
            case CTokenKind::Greater:
                return BinaryOperator::Gt;
            case CTokenKind::GreaterGreater:
                return BinaryOperator::Shr;
            case CTokenKind::Lesser:
                return BinaryOperator::Lt;
            case CTokenKind::LesserEqual:
                return BinaryOperator::Lte;
            case CTokenKind::LesserLesser:
                return BinaryOperator::Shl;
            case CTokenKind::EqualEqual:
                return BinaryOperator::Eq;
            case CTokenKind::AmpAmp:
                return BinaryOperator::Land;
            case CTokenKind::Amp:
                return BinaryOperator::And;
            case CTokenKind::PipePipe:
                return BinaryOperator::Lor;
            case CTokenKind::Pipe:
                return BinaryOperator::Or;
            case CTokenKind::Minus:
                return BinaryOperator::Sub;
            case CTokenKind::Plus:
                return BinaryOperator::Add;
            case CTokenKind::Percent:
                return BinaryOperator::Mod;
            case CTokenKind::Asterisk:
                return BinaryOperator::Mul;
            case CTokenKind::Slash:
                return BinaryOperator::Div;
            case CTokenKind::Caret:
                return BinaryOperator::Xor;
            default:
                return std::nullopt;
        }
    }

    // maps an assignment token to its operator. '=' maps to None, any
    // non-assignment token to nullopt.
    std::optional<AssignmentOperator> assign_op(const CTokenKind kind) {
        switch (kind) {
            case CTokenKind::Equal:
                return AssignmentOperator::None;
            case CTokenKind::PlusEqual:
                return AssignmentOperator::Add;
            case CTokenKind::MinusEqual:
                return AssignmentOperator::Sub;
            case CTokenKind::AsteriskEqual:
                return AssignmentOperator::Mul;
            case CTokenKind::SlashEqual:
                return AssignmentOperator::Div;
            case CTokenKind::PercentEqual:
                return AssignmentOperator::Mod;
            case CTokenKind::AmpEqual:
                return AssignmentOperator::And;
            case CTokenKind::PipeEqual:
                return AssignmentOperator::Or;
            case CTokenKind::CaretEqual:
                return AssignmentOperator::Xor;
            case CTokenKind::LesserLesserEqual:
                return AssignmentOperator::Shl;
            case CTokenKind::GreaterGreaterEqual:
                return AssignmentOperator::Shr;
            default:
                return std::nullopt;
        }
    }

}  // namespace
namespace Z::Zaban::Langs::CLang {
    using AssignmentExpression =
        Zaban::AST::Expressions::AssignmentExpression<COffsetType>;
    using BinaryExpression =
        Zaban::AST::Expressions::BinaryExpression<COffsetType>;
    using PrefixExpression =
        Zaban::AST::Expressions::PrefixExpression<COffsetType>;
    using SuffixExpression =
        Zaban::AST::Expressions::SuffixExpression<COffsetType>;

    CExpression CParser::parse_expression() const {
        return this->parse_assignment();
    }

    CExpression CParser::parse_assignment() const {
        auto lhs = this->parse_ternary();
        if (!lhs) {
            return nullptr;
        }

        const auto* tok = this->peek();
        if (!tok) {
            return lhs;
        }
        const auto op = assign_op(tok->kind);
        if (!op) {
            return lhs;
        }
        this->advance();

        // right associative: a = b = c is a = (b = c).
        auto rhs = this->parse_assignment();
        if (!rhs) {
            return nullptr;
        }

        return make_node<AssignmentExpression>(
            span(lhs->location(), rhs->location()), *op, lhs, rhs);
    }

    CExpression CParser::parse_ternary() const {
        auto condition = this->parse_logical_or();
        if (!condition) return nullptr;

        if (this->check(CTokenKind::Question)) {
            this->advance();
            auto true_expr = this->parse_ternary();
            if (!true_expr) return nullptr;

            if (!this->expect(CTokenKind::Colon,
                              CParserDiagnosticKind::ErrorExpectedToken)) {
                return nullptr;
            }

            auto false_expr = this->parse_assignment();
            if (!false_expr) return nullptr;

            return make_node<Z::Zaban::Langs::CLang::AST::Expressions::
                                 TernaryExpressionNode<COffsetType>>(
                span(condition->location(), false_expr->location()), condition,
                true_expr, false_expr);
        }
        return condition;
    }

    CExpression CParser::parse_logical_or() const {
        auto lhs = this->parse_logical_and();
        if (!lhs) return nullptr;

        while (this->check(CTokenKind::PipePipe)) {
            // TODO: improve this way of getting bin_op
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_logical_and();
            if (!rhs) return nullptr;
            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_logical_and() const {
        auto lhs = this->parse_bitwise_or();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::AmpAmp)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_bitwise_or();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_bitwise_or() const {
        auto lhs = this->parse_bitwise_xor();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::Pipe)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_bitwise_xor();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_bitwise_xor() const {
        auto lhs = this->parse_bitwise_and();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::Caret)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_bitwise_and();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_bitwise_and() const {
        auto lhs = this->parse_equality();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::Amp)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_equality();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_equality() const {
        auto lhs = this->parse_comparison();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::EqualEqual) ||
               this->check(CTokenKind::PipeEqual)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_comparison();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_comparison() const {
        auto lhs = this->parse_shifting();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::Greater) ||
               this->check(CTokenKind::GreaterEqual) ||
               this->check(CTokenKind::LesserEqual) ||
               this->check(CTokenKind::Lesser)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_shifting();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_shifting() const {
        auto lhs = this->parse_additive();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::GreaterGreater) ||
               this->check(CTokenKind::LesserLesser)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_additive();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_additive() const {
        auto lhs = this->parse_multipicative();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::Plus) ||
               this->check(CTokenKind::Minus)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_multipicative();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_multipicative() const {
        auto lhs = this->parse_unary();
        if (!lhs) return nullptr;
        while (this->check(CTokenKind::Asterisk) ||
               this->check(CTokenKind::Slash) ||
               this->check(CTokenKind::Percent)) {
            auto bin_op = assign_bin_op(this->peek()->kind).value();
            this->advance();
            auto rhs = this->parse_unary();
            if (!rhs) return nullptr;

            lhs = make_node<BinaryExpression>(
                span(lhs->location(), rhs->location()), lhs, rhs, bin_op);
        }
        return lhs;
    }

    CExpression CParser::parse_unary() const {
        const auto* tok = this->peek();
        if (!tok) return this->parse_suffix();

        const auto op = assign_prefix_op(tok->kind);
        if (!op) return this->parse_suffix();
        this->advance();

        // prefix operators nest right to left: --x is -(-x), *&p is *(&p).
        auto operand = this->parse_unary();
        if (!operand) return nullptr;

        return make_node<PrefixExpression>(
            span(tok->range, operand->location()), operand, *op);
    }

    CExpression CParser::parse_suffix() const {
        auto operand = this->parse_primary();
        if (!operand) return nullptr;

        // suffix operators nest left to right: a[i]++ is (a[i])++.
        // TODO: calls f(x), indexing a[i] and member access .x/->x go here
        while (const auto* tok = this->peek()) {
            const auto op = assign_suffix_op(tok->kind);
            if (!op) break;
            this->advance();

            operand = make_node<SuffixExpression>(
                span(operand->location(), tok->range), operand, *op);
        }
        return operand;
    }

    CExpression CParser::parse_primary() const {
    }

    CExpression CParser::parse_group() const {
    }
}  // namespace Z::Zaban::Langs::CLang

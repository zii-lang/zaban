
#include <Z/Zaban/AST/Expressions/AssignmentExpression.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/ParserDiagnostic.hpp>
#include <memory>
#include <optional>

#include "Z/Zaban/AST/Expression.hpp"
#include "Z/Zaban/Langs/CLang/AST/Expressions/TernaryExpression.hpp"

namespace {

    using CTokenKind         = Z::Zaban::Langs::CLang::CTokenKind;
    using AssignmentOperator = Z::Zaban::AST::Expressions::AssignmentOperator;
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
            auto true_expr = this->parse_assignment();
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
    }

    CExpression CParser::parse_logical_and() const {
    }

    CExpression CParser::parse_bitwise_or() const {
    }

    CExpression CParser::parse_bitwise_xor() const {
    }

    CExpression CParser::parse_bitwise_and() const {
    }

    CExpression CParser::parse_equality() const {
    }

    CExpression CParser::parse_comparison() const {
    }

    CExpression CParser::parse_shifting() const {
    }

    CExpression CParser::parse_additive() const {
    }

    CExpression CParser::parse_multipicative() const {
    }

    CExpression CParser::parse_unary() const {
    }

    CExpression CParser::parse_suffix() const {
    }

    CExpression CParser::parse_primary() const {
    }

    CExpression CParser::parse_group() const {
    }
}  // namespace Z::Zaban::Langs::CLang

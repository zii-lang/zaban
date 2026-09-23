#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Identifies a postfix (suffix) unary operator.
     *
     * Suffix operators are applied after an expression and operate on the
     * expression's current value. They typically modify the value after it has
     * been evaluated.
     */
    enum class SuffixOperator {
        /// Post-increment operator (`++`).
        AddAdd,
        /// Post-decrement operator (`--`).
        SubSub,
    };

    /** @brief Represents a postfix operation expression.
     *
     * SuffixExpression represents an expression followed by a postfix operator,
     * where the operand is evaluated with the operator applied after its value
     * is used.
     */
    template<typename OffsetType = std::size_t>
    class SuffixExpression : public ExpressionNode {
        const SuffixOperator _op;
        const Expression     _expr;

       public:
        /** @brief Creates a postfix expression. */
        SuffixExpr(Expression expr, SuffixOp operator) :
            _expr(std::move(expr)), _op(operator) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Suffix;
        }

        /** @brief Returns the postfix operator. */
        SuffixOperator get_operator() const {
            return this->_op;
        }

        /** @brief Returns the operand expression. */
        Expression get_expr() const {
            return this->_expr;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

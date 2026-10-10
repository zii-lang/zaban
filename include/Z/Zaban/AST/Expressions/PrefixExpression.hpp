#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Identifies a prefix unary operator.
     *
     * Prefix operators are applied before an expression is evaluated. They may
     * transform the value, perform a logical operation, or access a different
     * representation of the operand.
     */
    enum class PrefixOperator {
        /// Pre-increment operator (`++`).
        AddAdd,
        /// Pre-decrement operator (`--`).
        SubSub,
        /// Arithmetic negation operator (`-`).
        Neg,
        /// Unary plus operator (`+`).
        ///
        /// Leaves the value unchanged apart from integer promotion.
        Pos,
        /// Bitwise NOT operator (`~`).
        BNeg,
        /// Logical NOT operator (`!`).
        LNeg,
        /// Dereference operator (`*>`).
        ///
        /// Resolves a pointer value to access the referenced object.
        Deref,
        /// Address-of operator (`&>`).
        ///
        /// Produces the address of an expression.
        AddrOf,
    };

    /** @brief Represents a prefix operation expression.
     *
     * PrefixExpression represents an expression with a prefix operator applied
     * before evaluating the operand.
     */
    template<typename OffsetType = std::size_t>
    class PrefixExpression : public ExpressionNode<OffsetType> {
        const PrefixOperator         _op;
        const Expression<OffsetType> _expr;

       public:
        /** @brief Creates a prefix expression. */
        PrefixExpression(Expression<OffsetType> expr, PrefixOperator op) :
            _op(op), _expr(std::move(expr)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Prefix;
        }

        /** @brief Returns the prefix operator. */
        PrefixOperator get_operator() const {
            return this->_op;
        }

        /** @brief Returns the operand expression. */
        Expression<OffsetType> get_expr() const {
            return this->_expr;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

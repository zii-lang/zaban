#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Identifies a binary operation.
     *
     * Binary operators combine two operands to produce a resulting value. They
     * include arithmetic, bitwise, shift, comparison, and logical operations.
     */
    enum class BinaryOperator {
        /// Multiplication operator (`*`).
        Mul,
        /// Division operator (`/`).
        Div,
        /// Remainder/modulo operator (`%`).
        Mod,
        /// Addition operator (`+`).
        Add,
        /// Subtraction operator (`-`).
        Sub,
        /// Left shift operator (`<<`).
        Shl,
        /// Right shift operator (`>>`).
        Shr,
        /// Less-than comparison operator (`<`).
        Lt,
        /// Greater-than comparison operator (`>`).
        Gt,
        /// Less-than-or-equal comparison operator (`<=`).
        Lte,
        /// Greater-than-or-equal comparison operator (`>=`).
        Gte,
        /// Equality comparison operator (`==`).
        Eq,
        /// Inequality comparison operator (`!=`).
        Neq,
        /// Bitwise AND operator (`&`).
        And,
        /// Bitwise XOR operator (`^`).
        Xor,
        /// Bitwise OR operator (`|`).
        Or,
        /// Logical AND operator (`&&`).
        Land,
        /// Logical OR operator (`||`).
        Lor,
    };

    /** @brief Represents a binary operation expression.
     *
     * BinaryExpr stores an operator and two operand expressions. The operation
     * is evaluated by applying the operator to the left and right expressions.
     */
    template<typename OffsetType = std::size_t>
    class BinaryExpression : public ExpressionNode {
       private:
        const BinaryOperator _op;
        const Expression     _left;
        const Expression     _right;

       public:
        /** @brief Creates a binary expression. */
        BinaryExpression(Expr left, Expr right, BinaryOp op) :
            _op(op), _left(std::move(left)), _right(std::move(right)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Binary;
        }

        /** @brief Returns the binary operator. */
        BinaryOperator get_op() const {
            return this->_op;
        }

        /** @brief Returns the left operand expression. */
        const Expression get_left() const {
            return this->_left;
        }

        /** @brief Returns the right operand expression. */
        const Expression get_right() const {
            return this->_right;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

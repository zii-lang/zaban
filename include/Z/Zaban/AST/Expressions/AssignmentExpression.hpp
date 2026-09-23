#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Identifies the type of assignment operation.
     *
     * An AssignmentOperator represents the operation performed when assigning
     * a value to an existing storage location. It distinguishes between a
     * simple assignment and compound assignments, where the right-hand side
     * value is combined with the current value before being stored.
     *
     * Compound assignment operators are equivalent to applying a binary
     * operator followed by a normal assignment:
     *
     * @code
     * a += b  ->  a = a + b
     * a *= b  ->  a = a * b
     * @endcode
     */
    enum class AssignmentOperator {
        /// Simple assignment operator (`=`).
        None,
        /// Addition assignment operator (`+=`).
        Add,
        /// Subtraction assignment operator (`-=`).
        Sub,
        /// Multiplication assignment operator (`*=`).
        Mul,
        /// Division assignment operator (`/=`).
        Div,
        /// Remainder/modulo assignment operator (`%=`).
        Mod,
        /// Bitwise OR assignment operator (`|=`).
        Or,
        /// Bitwise AND assignment operator (`&=`).
        And,
        /// Bitwise XOR assignment operator (`^=`).
        Xor,
        /// Left shift assignment operator (`<<=`).
        Shl,
        /// Right shift assignment operator (`>>=`).
        Shr,
    };

    /** @brief Represents an assignment expression.
     *
     * AssignmentExpr represents assigning a value or type annotation to a
     * target expression.
     *
     * The right-hand side may contain either an expression value or an
     * annotation, allowing assignments to represent both runtime value
     * assignment and type-related assignment forms.
     */
    template<typename OffsetType = std::size_t>
    class AssignmentExpression : public ExpressionNode {
        const AssignmentOperator                   _op;
        const Expression                           _left;
        const std::variant<Expression, Annotation> _right;

       public:
        /** @brief Creates an assignment expression with an expression value. */
        AssignmentExpr(AssignmentOperator operation, Expression left,
                       Expression right) :
            _op(operation), _left(std::move(left)), _right(std::move(right)) {
        }

        /** @brief Creates an assignment expression with a type annotation. */
        AssignmentExpr(AssignmentOperator operation, Expression left,
                       Annotation right) :
            _op(operation), _left(std::move(left)), _right(std::move(right)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Assignment;
        }

        /** @brief Returns the assignment operator. */
        AssignmentOperator get_operation() const {
            return this->_op;
        }

        /** @brief Returns the assignment target expression. */
        Expression get_left() const {
            return this->_left;
        }

        /** @brief Returns the assigned value or annotation. */
        std::variant<Expression, Annotation> get_right() const {
            return this->_right;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

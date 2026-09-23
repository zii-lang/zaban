#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Represents an indexed access expression.
     *
     * IndexAccessExpression represents accessing an element from an indexed
     * value, such as an array or other indexable object.
     *
     * Example:
     * @code
     * values[index]
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class IndexAccessExpression : public ExpressionNode {
       private:
        const Expression _expr;
        const Expression _access;

       public:
        /** @brief Creates an indexed access expression. */
        IndexAccessExpression(Expression expr, Expression access) :
            _expr(std::move(expr)), _access(std::move(access)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::IndexAccess;
        };

        /** @brief Returns the expression being indexed. */
        Expression expr() const {
            return this->_expr;
        }

        /** @brief Returns the index expression. */
        Expression access() const {
            return this->_access;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

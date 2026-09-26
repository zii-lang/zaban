#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Represents a parenthesized expression.
     *
     * Stores an inner expression while preserving grouping information from the
     * source code.
     */
    template<typename OffsetType = std::size_t>
    class GroupExpression : public ExpressionNode {
        const Expression value;

       public:
        /** @brief Creates a grouped expression. */
        GroupExpression(Expr&& expr) : value(expr) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Group;
        }

        /** @brief Returns the inner expression. */
        Expression get() const {
            return this->value;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

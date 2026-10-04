#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Represents a parenthesized expression.
     *
     * Stores an inner expression while preserving grouping information from the
     * source code.
     */
    template<typename OffsetType = std::size_t>
    class GroupExpression : public ExpressionNode<OffsetType> {
        const Expression<OffsetType> m_inner;

       public:
        /** @brief Creates a grouped expression. */
        GroupExpression(Expression<OffsetType>&& expr) : m_inner(expr) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Group;
        }

        /** @brief Returns the inner expression. */
        Expression<OffsetType> inner() const {
            return this->m_inner;
        }

        Expression<OffsetType> get_ptr() {
            return std::make_shared<GroupExpression<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Expressions

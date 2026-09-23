#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    /** @brief Represents a member access expression.
     *
     * MemberAccessExpression represents accessing a named member from a base
     * expression.
     *
     * Example:
     * @code
     * base.member
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class MemberAccessExpression : public ExpressionNode {
        Expression _base;
        Atomic     _member;

       public:
        /** @brief Creates a member access expression. */
        MemberAccessExpression(Expr base, Atomic member) :
            _base(std::move(base)), _member(member) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::MemberAccess;
        }

        /** @brief Returns the base expression. */
        Expression get_base() const {
            return this->_base;
        }

        /** @brief Returns the accessed member. */
        Atomic get_member() const {
            return this->_member;
        }
    };
}  // namespace Z::Zaban::AST::Expressions

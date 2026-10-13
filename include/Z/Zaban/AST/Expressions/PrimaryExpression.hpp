#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <memory>

namespace Z::Zaban::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class PrimaryExpression : public ExpressionNode<OffsetType> {
        const Atomic<OffsetType> m_value;

       public:
        /** @brief Creates a primary expression from an identifier. */
        PrimaryExpression(Atomic<OffsetType> value) : m_value(value) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Primary;
        }

        /** @brief Returns the stored primary value as the requested type.
         *
         * The requested type must match the active alternative stored in the
         * primary expression value.
         */
        template<typename T>
        std::shared_ptr<T> get() const {
            return std::static_pointer_cast<T>(this->value);
        }

        Expression<OffsetType> get_ptr() {
            return std::make_shared<PrimaryExpression<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Expressions

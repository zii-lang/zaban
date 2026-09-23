#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class PrimaryExpression : public ExpressionNode {
        const Atomic value;

       public:
        /** @brief Creates a primary expression from an identifier. */
        PrimaryExpression(Identifier&& id) : value(id) {
        }

        /** @brief Creates a primary expression from a literal. */
        PrimaryExpression(Literal&& literal) : value(literal) {
        }

        /** @brief Creates a primary expression from an identifier name. */
        PrimaryExpression(std::string&& id) :
            value(std::make_shared<IdentifierNode>(id)) {
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
        T get() const {
            return std::get<T>(this->value);
        }
    };
}  // namespace Z::Zaban::AST::Expressions

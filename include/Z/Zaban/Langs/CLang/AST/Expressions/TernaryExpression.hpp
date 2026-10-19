#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <memory>

namespace Z::Zaban::Langs::CLang::AST::Expressions {
    /** @brief Represents a C conditional (ternary) expression.
     *
     * Evaluates the condition and yields the true expression when it is
     * non-zero, or the false expression otherwise. The operator is right
     * associative.
     *
     * Example:
     * @code
     * a > b ? a : b
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class TernaryExpressionNode : public Zaban::AST::ExpressionNode<OffsetType> {
        const Zaban::AST::Expression<OffsetType> _condition;
        const Zaban::AST::Expression<OffsetType> _true_expr;
        const Zaban::AST::Expression<OffsetType> _false_expr;

       public:
        /** @brief Creates a ternary expression. */
        TernaryExpressionNode(Zaban::AST::Expression<OffsetType> condition,
                              Zaban::AST::Expression<OffsetType> true_expr,
                              Zaban::AST::Expression<OffsetType> false_expr) :
            _condition(std::move(condition)), _true_expr(std::move(true_expr)),
            _false_expr(std::move(false_expr)) {
        }

        /** @brief Returns the expression kind. */
        Zaban::AST::ExpressionKind expr_kind() const override {
            return Zaban::AST::ExpressionKind::Conditional;
        }

        /** @brief Returns the condition expression. */
        Zaban::AST::Expression<OffsetType> get_condition() const {
            return this->_condition;
        }

        /** @brief Returns the expression yielded when the condition holds. */
        Zaban::AST::Expression<OffsetType> get_true() const {
            return this->_true_expr;
        }

        /** @brief Returns the expression yielded otherwise. */
        Zaban::AST::Expression<OffsetType> get_false() const {
            return this->_false_expr;
        }
    };

    template<typename OffsetType = std::size_t>
    using TernaryExpression =
        std::shared_ptr<TernaryExpressionNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Expressions

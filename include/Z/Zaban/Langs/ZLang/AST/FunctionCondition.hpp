#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/HalfCondition.hpp>
#include <optional>
#include <vector>

namespace Z::Zaban::Langs::ZLang::AST {
    /** @brief Represents a function-based condition expression.
     *
     * FunctionCondition represents a conditional check performed by calling a
     * function with arguments derived from the condition inputs.
     *
     * The argument passing mode controls how condition values are provided to
     * the function:
     *
     * - PassAll: Passes all available condition arguments in order.
     * - Positional: Passes selected arguments by index.
     * - Exact: Passes explicitly provided expressions.
     *
     * A condition may optionally include a comparison that evaluates the
     * returned function value.
     *
     * Example:
     * @code
     * if a, b
     *     ?? [] check == true => { ... }
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class FunctionCondition {
        ConditionArgMode         _mode;
        std::vector<std::size_t> _positional_indices;  // for positional mode
        std::vector<Expression>  _exact_args;          // for exact mode
        Expression               _callee;
        std::optional<HalfCondition> _comparison;

       public:
        /** @brief Creates a function condition with the specified argument
         * mode. */
        FunctionCondition(ConditionArgMode mode, Expr&& callee) :
            _mode(mode), _callee(std::move(callee)), _comparison(std::nullopt) {
        }

        /** @brief Creates a function condition using positional arguments. */
        FunctionCondition(std::vector<size_t> indices, Expr&& callee) :
            _mode(ConditionArgMode::Positional),
            _positional_indices(std::move(indices)), _callee(std::move(callee)),
            _comparison(std::nullopt) {
        }

        /** @brief Creates a function condition using explicit arguments. */
        FunctionCondition(std::vector<Expr> args, Expr&& callee) :
            _mode(ConditionArgMode::Exact), _exact_args(std::move(args)),
            _callee(std::move(callee)) {
        }

        /** @brief Creates a function condition with a comparison. */
        FunctionCondition(ConditionArgMode mode, Expr&& callee,
                          HalfCondition cmp) :
            _mode(mode), _callee(std::move(callee)),
            _comparison(std::move(cmp)) {
        }

        /** @brief Creates a positional function condition with a comparison. */
        FunctionCondition(std::vector<size_t> indices, Expr&& callee,
                          HalfCondition cmp) :
            _mode(ConditionArgMode::Positional),
            _positional_indices(std::move(indices)), _callee(std::move(callee)),
            _comparison(std::move(cmp)) {
        }

        /** @brief Creates an exact argument function condition with a
         * comparison. */
        FunctionCondition(std::vector<Expr> args, Expr callee,
                          HalfCondition cmp) :
            _mode(ConditionArgMode::Exact), _exact_args(std::move(args)),
            _callee(std::move(callee)), _comparison(std::move(cmp)) {
        }

        /** @brief Returns the argument passing mode. */
        ConditionArgMode get_mode() const {
            return _mode;
        }

        /** @brief Returns positional argument indices. */
        const std::vector<size_t>& get_positional_indices() const {
            return _positional_indices;
        }

        /** @brief Returns explicitly specified arguments. */
        const std::vector<Expr>& get_exact_args() const {
            return _exact_args;
        }

        /** @brief Returns the function expression being called. */
        Expr get_callee() const {
            return _callee;
        }

        /** @brief Returns whether this condition has a comparison. */
        bool has_comparison() const {
            return _comparison.has_value();
        }

        /** @brief Returns the comparison applied to the function result. */
        const HalfCondition& get_comparison() const {
            return _comparison.value();
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST

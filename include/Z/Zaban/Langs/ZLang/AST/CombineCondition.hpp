#pragma once

#include <Z/Zaban/Langs/ZLang/AST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/HalfCondition.hpp>

namespace Z::Zaban::Langs::ZLang::AST {
    /** @brief Represents a condition combined with a condition combinator.
     *
     * CombinatorHalfConditionBase associates a half-condition with a combinator
     * operator that defines how it participates in a conditional expression.
     *
     * Example:
     * @code
     * ?? == 20 ?& == 30
     * @endcode
     *
     * Each condition part is stored together with its logical combination mode,
     * such as AND or OR.
     */
    template<typename OffsetType = std::size_t>
    class CombineCondition {
        const HalfCondition               _cond;
        const ConditionCombinatorOperator _op;

       public:
        /** @brief Creates a combinator condition from an operator and
         * condition. */
        CombineCondition(ConditionCombinatorOperator op, HalfCondition&& half) :
            _op(op), _cond(std::move(half)) {
        }

        /** @brief Returns the condition combinator operator. */
        ConditionCombinatorOperator get_operator() const {
            return this->_op;
        }

        /** @brief Returns the associated half-condition. */
        HalfCondition get_condition() const {
            return this->_cond;
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST

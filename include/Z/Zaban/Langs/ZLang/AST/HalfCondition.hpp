#pragma once

#include <Z/Zaban/AST/Condition.hpp>
#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::Langs::ZLang::AST {
    /** @brief Represents a single boolean condition check.
     *
     * HalfCondition stores a comparison operation together with its operands.
     * It can represent both unary-style condition checks, where only the
     * right-hand expression is evaluated, and binary comparisons involving a
     * left and right expression.
     *
     * Example:
     * @code
     * == value
     * a == value
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class HalfCondition {
        AST::BooleanOperator _op;
        AST::Expression      _rhs;
        AST::Expression      _lhs = nullptr;

       public:
        /** @brief Creates a condition with only a right-hand expression. */
        HalfCondition(AST::BooleanOperator op, AST::Expression&& rhs) :
            _op(op), _rhs(std::move(rhs)) {
        }

        /** @brief Creates a condition with left and right expressions. */
        HalfCondition(AST::BooleanOperator op, AST::Expression&& lhs,
                      AST::Expression&& rhs) :
            _op(op), _rhs(std::move(rhs)), _lhs(std::move(lhs)) {
        }

        /** @brief Returns the comparison operator. */
        AST::BooleanOperator get_operation() const {
            return this->_op;
        }

        /** @brief Returns the right-hand expression. */
        AST::Expression get_rhs() const {
            return this->_rhs;
        }

        /** @brief Returns the left-hand expression, if present. */
        AST::Expression get_lhs() const {
            return this->_lhs;
        }

        /** @brief Returns the expression used as the primary condition value.
         */
        AST::Expression get_expr() const {
            return this->_rhs;
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST

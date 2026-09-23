#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/HalfCondition.hpp>

namespace Z::Zaban::Langs::ZLang::AST {
    template<typename OffsetType = std::size_t>
    class ConditionLineNode : public Zaban::AST::Node {
        ConditionLineOperator                _op;
        std::optional<HalfCondition>         _base;
        std::optional<FunctionCondition>     _func_cond;
        std::vector<CombinatorHalfCondition> _comb;
        Statement                            _stmt;

       public:
        /** @brief Creates a condition line without a condition. */
        ConditionLineNode(ConditionLineOperator op, Statement&& stmt) :
            _op(op), _stmt(std::move(stmt)), _base(std::nullopt), _comb() {
        }

        /** @brief Creates a condition line with a base condition. */
        ConditionLineNode(ConditionLineOperator op, Statement&& stmt,
                          HalfCondition&& base) :
            _op(op), _stmt(std::move(stmt)), _base(std::move(base)), _comb() {
        }

        /** @brief Creates a condition line with a base and combinator
         * conditions. */
        ConditionLineNode(ConditionLineOperator op, Statement&& stmt,
                          HalfCondition&&                        base,
                          std::vector<CombinatorHalfCondition>&& comb) :
            _op(op), _stmt(std::move(stmt)), _base(std::move(base)),
            _comb(std::move(comb)) {
        }

        /** @brief Creates a condition line with a function condition. */
        ConditionLineNode(ConditionLineOperator op, Statement&& stmt,
                          FunctionCondition&& func_cond) :
            _op(op), _stmt(std::move(stmt)), _func_cond(std::move(func_cond)),
            _base(std::nullopt), _comb() {
        }

        /** @brief Returns the condition line operation mode. */
        ConditionLineOperator get_operation() const {
            return this->_op;
        }

        /** @brief Returns the statement executed by this condition line. */
        Statement get_statement() const {
            return this->_stmt;
        }

        /** @brief Returns whether this line has a base condition. */
        bool has_base() const {
            return this->_base.has_value();
        }

        /** @brief Returns whether this line has a function condition. */
        bool has_func_cond() const {
            return this->_func_cond.has_value();
        }

        /** @brief Returns the function condition. */
        const FunctionCondition& get_func_cond() const {
            return _func_cond.value();
        }

        /** @brief Returns the base condition. */
        std::optional<HalfCondition> get_base() const {
            return this->_base.value();
        }

        /** @brief Returns the number of combinator conditions. */
        std::size_t get_combc() const {
            return this->_comb.size();
        }

        /** @brief Returns a combinator condition by index. */
        CombinatorHalfCondition get_comb_at(std::size_t pos) {
            return this->_comb[pos];
        }

        /** @brief Returns an iterator to the first combinator condition. */
        std::vector<CombinatorHalfCondition>::iterator comb_begin() {
            return this->_comb.begin();
        }

        /** @brief Returns an iterator past the last combinator condition. */
        std::vector<CombinatorHalfCondition>::iterator comb_end() {
            return this->_comb.end();
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST

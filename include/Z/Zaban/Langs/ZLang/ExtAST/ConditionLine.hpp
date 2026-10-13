#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/CombineCondition.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/FunctionCondition.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/HalfCondition.hpp>
#include <optional>
#include <vector>

namespace Z::Zaban::Langs::ZLang::ExtAST {
    /** @brief Represents a single conditional branch line.
     *
     * ConditionLine represents one branch of a conditional statement. A
     * condition line may contain a direct condition, a function-based
     * condition, optional combinator conditions, and the statement executed
     * when the condition matches.
     *
     * The operation determines how this line participates in the surrounding
     * conditional structure:
     *
     * - Canon: regular conditional branch.
     * - Serial: evaluates as an independent conditional branch.
     * - Parallel: evaluates concurrently with other branches.
     */
    template<typename OffsetType = std::size_t>
    class ConditionLineNode : public AST::Node<OffsetType> {
        ConditionLineOperator                        _op;
        std::optional<HalfCondition<OffsetType>>     _base;
        std::optional<FunctionCondition<OffsetType>> _func_cond;
        std::vector<CombineCondition<OffsetType>>    _comb;
        Z::Zaban::AST::Statement<OffsetType>         _stmt;

       public:
        /** @brief Creates a condition line without a condition. */
        ConditionLineNode(ConditionLineOperator                  op,
                          Z::Zaban::AST::Statement<OffsetType>&& stmt) :
            _op(op), _stmt(std::move(stmt)), _base(std::nullopt), _comb() {
        }

        /** @brief Creates a condition line with a base condition. */
        ConditionLineNode(ConditionLineOperator                  op,
                          Z::Zaban::AST::Statement<OffsetType>&& stmt,
                          HalfCondition<OffsetType>&&            base) :
            _op(op), _stmt(std::move(stmt)), _base(std::move(base)), _comb() {
        }

        /** @brief Creates a condition line with a base and combinator
         * conditions. */
        ConditionLineNode(ConditionLineOperator                       op,
                          Z::Zaban::AST::Statement<OffsetType>&&      stmt,
                          HalfCondition<OffsetType>&&                 base,
                          std::vector<CombineCondition<OffsetType>>&& comb) :
            _op(op), _stmt(std::move(stmt)), _base(std::move(base)),
            _comb(std::move(comb)) {
        }

        /** @brief Creates a condition line with a function condition. */
        ConditionLineNode(ConditionLineOperator                  op,
                          Z::Zaban::AST::Statement<OffsetType>&& stmt,
                          FunctionCondition<OffsetType>&&        func_cond) :
            _op(op), _stmt(std::move(stmt)), _func_cond(std::move(func_cond)),
            _base(std::nullopt), _comb() {
        }

        /** @brief Returns the condition line operation mode. */
        ConditionLineOperator get_operation() const {
            return this->_op;
        }

        /** @brief Returns the statement executed by this condition line. */
        Z::Zaban::AST::Statement<OffsetType> get_statement() const {
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
        const FunctionCondition<OffsetType>& get_func_cond() const {
            return _func_cond.value();
        }

        /** @brief Returns the base condition. */
        std::optional<HalfCondition<OffsetType>> get_base() const {
            return this->_base.value();
        }

        /** @brief Returns the number of combinator conditions. */
        std::size_t get_combc() const {
            return this->_comb.size();
        }

        /** @brief Returns a combinator condition by index. */
        CombineCondition<OffsetType> get_comb_at(std::size_t pos) {
            return this->_comb[pos];
        }

        /** @brief Returns an iterator to the first combinator condition. */
        std::vector<CombineCondition<OffsetType>>::iterator comb_begin() {
            return this->_comb.begin();
        }

        /** @brief Returns an iterator past the last combinator condition. */
        std::vector<CombineCondition<OffsetType>>::iterator comb_end() {
            return this->_comb.end();
        }
    };

    template<typename OffsetType = std::size_t>
    using ConditionLine = std::shared_ptr<ConditionLineNode<OffsetType>>;

}  // namespace Z::Zaban::Langs::ZLang::ExtAST

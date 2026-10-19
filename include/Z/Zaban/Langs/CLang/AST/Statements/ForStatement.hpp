#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/LoopStatement.hpp>
#include <memory>

namespace Z::Zaban::Langs::CLang::AST::Statements {
    /** @brief Represents a C for loop.
     *
     * The init clause is a statement so that it can hold either a declaration
     * or an expression statement. Every clause is optional; a missing
     * condition loops forever.
     *
     * Example:
     * @code
     * for (int i = 0; i < 10; i++) sum += i;
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class ForStatementNode : public LoopStatementNode<OffsetType> {
        // Optional init clause (declaration or expression statement).
        const Zaban::AST::Statement<OffsetType> _init;

        // Optional condition.
        const Zaban::AST::Expression<OffsetType> _condition;

        // Optional step expression.
        const Zaban::AST::Expression<OffsetType> _step;

        const Zaban::AST::Statement<OffsetType> _body;

       public:
        /** @brief Creates a for loop. Pass nullptr for omitted clauses. */
        ForStatementNode(Zaban::AST::Statement<OffsetType>  init,
                         Zaban::AST::Expression<OffsetType> condition,
                         Zaban::AST::Expression<OffsetType> step,
                         Zaban::AST::Statement<OffsetType>  body) :
            _init(std::move(init)), _condition(std::move(condition)),
            _step(std::move(step)), _body(std::move(body)) {
        }

        /** @brief Returns the loop kind. */
        LoopKind loop_kind() const override {
            return LoopKind::For;
        }

        /** @brief Returns the init clause, if available. */
        Zaban::AST::Statement<OffsetType> get_init() const {
            return this->_init;
        }

        /** @brief Returns the loop condition, if available. */
        Zaban::AST::Expression<OffsetType> get_condition() const {
            return this->_condition;
        }

        /** @brief Returns the step expression, if available. */
        Zaban::AST::Expression<OffsetType> get_step() const {
            return this->_step;
        }

        /** @brief Returns the loop body. */
        Zaban::AST::Statement<OffsetType> get_body() const {
            return this->_body;
        }
    };

    template<typename OffsetType = std::size_t>
    using ForStatement = std::shared_ptr<ForStatementNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Statements

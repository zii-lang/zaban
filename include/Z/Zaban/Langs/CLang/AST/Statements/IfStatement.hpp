#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <memory>

namespace Z::Zaban::Langs::CLang::AST::Statements {
    /** @brief Represents a C if statement.
     *
     * IfStatementNode evaluates a condition and executes the then branch when
     * it is non-zero, or the optional else branch otherwise. An else-if chain
     * is represented by an IfStatementNode nested as the else branch.
     *
     * Example:
     * @code
     * if (a > 0) b = 1; else b = 2;
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class IfStatementNode : public Zaban::AST::StatementNode<OffsetType> {
        const Zaban::AST::Expression<OffsetType> _condition;
        const Zaban::AST::Statement<OffsetType>  _then_branch;

        // Optional else branch.
        const Zaban::AST::Statement<OffsetType> _else_branch = nullptr;

       public:
        /** @brief Creates an if statement without an else branch. */
        IfStatementNode(Zaban::AST::Expression<OffsetType> condition,
                        Zaban::AST::Statement<OffsetType>  then_branch) :
            _condition(std::move(condition)),
            _then_branch(std::move(then_branch)) {
        }

        /** @brief Creates an if statement with an else branch. */
        IfStatementNode(Zaban::AST::Expression<OffsetType> condition,
                        Zaban::AST::Statement<OffsetType>  then_branch,
                        Zaban::AST::Statement<OffsetType>  else_branch) :
            _condition(std::move(condition)),
            _then_branch(std::move(then_branch)),
            _else_branch(std::move(else_branch)) {
        }

        /** @brief Returns the statement kind. */
        Zaban::AST::StatementKind statement_kind() const override {
            return Zaban::AST::StatementKind::Conditional;
        }

        /** @brief Returns the condition expression. */
        Zaban::AST::Expression<OffsetType> get_condition() const {
            return this->_condition;
        }

        /** @brief Returns the branch executed when the condition holds. */
        Zaban::AST::Statement<OffsetType> get_then() const {
            return this->_then_branch;
        }

        /** @brief Returns whether this statement has an else branch. */
        bool has_else() const {
            return this->_else_branch != nullptr;
        }

        /** @brief Returns the else branch, if available. */
        Zaban::AST::Statement<OffsetType> get_else() const {
            return this->_else_branch;
        }
    };

    template<typename OffsetType = std::size_t>
    using IfStatement = std::shared_ptr<IfStatementNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Statements

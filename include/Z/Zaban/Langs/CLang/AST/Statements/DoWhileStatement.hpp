#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/LoopStatement.hpp>
#include <memory>

namespace Z::Zaban::Langs::CLang::AST::Statements {
    /** @brief Represents a C do-while loop.
     *
     * The condition is evaluated after each iteration, so the body executes at
     * least once.
     *
     * Example:
     * @code
     * do { i++; } while (i < 10);
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class DoWhileStatementNode : public LoopStatementNode<OffsetType> {
        const Zaban::AST::Statement<OffsetType>  _body;
        const Zaban::AST::Expression<OffsetType> _condition;

       public:
        /** @brief Creates a do-while loop. */
        DoWhileStatementNode(Zaban::AST::Statement<OffsetType>  body,
                             Zaban::AST::Expression<OffsetType> condition) :
            _body(std::move(body)), _condition(std::move(condition)) {
        }

        /** @brief Returns the loop kind. */
        LoopKind loop_kind() const override {
            return LoopKind::DoWhile;
        }

        /** @brief Returns the loop body. */
        Zaban::AST::Statement<OffsetType> get_body() const {
            return this->_body;
        }

        /** @brief Returns the loop condition. */
        Zaban::AST::Expression<OffsetType> get_condition() const {
            return this->_condition;
        }
    };

    template<typename OffsetType = std::size_t>
    using DoWhileStatement = std::shared_ptr<DoWhileStatementNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Statements

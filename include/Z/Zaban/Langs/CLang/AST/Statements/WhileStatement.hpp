#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/LoopStatement.hpp>
#include <memory>

namespace Z::Zaban::Langs::CLang::AST::Statements {
    /** @brief Represents a C while loop.
     *
     * The condition is evaluated before each iteration, so the body may not
     * execute at all.
     *
     * Example:
     * @code
     * while (i < 10) i++;
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class WhileStatementNode : public LoopStatementNode<OffsetType> {
        const Zaban::AST::Expression<OffsetType> _condition;
        const Zaban::AST::Statement<OffsetType>  _body;

       public:
        /** @brief Creates a while loop. */
        WhileStatementNode(Zaban::AST::Expression<OffsetType> condition,
                           Zaban::AST::Statement<OffsetType>  body) :
            _condition(std::move(condition)), _body(std::move(body)) {
        }

        /** @brief Returns the loop kind. */
        LoopKind loop_kind() const override {
            return LoopKind::While;
        }

        /** @brief Returns the loop condition. */
        Zaban::AST::Expression<OffsetType> get_condition() const {
            return this->_condition;
        }

        /** @brief Returns the loop body. */
        Zaban::AST::Statement<OffsetType> get_body() const {
            return this->_body;
        }
    };

    template<typename OffsetType = std::size_t>
    using WhileStatement = std::shared_ptr<WhileStatementNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Statements

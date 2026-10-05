#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST::Statements {
    template<typename OffsetType = std::size_t>
    class ExpressionStatement : public StatementNode<OffsetType> {
        const Expression<OffsetType> _expression;

       public:
        /** @brief Creates an expression statement. */
        ExpressionStatement(Expression<OffsetType>&& expression) :
            _expression(std::move(expression)) {
        }

        StatementKind statement_kind() const override {
            return StatementKind::Expression;
        }

        /** @brief Returns the contained expression. */
        const Expression<OffsetType> get() const {
            return this->_expression;
        }
    };
}  // namespace Z::Zaban::AST::Statements

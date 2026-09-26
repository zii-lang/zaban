#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <vector>

namespace Z::Zaban::AST::Expressions {
    /** @brief Represents a call expression.
     *
     * CallAccessExpression represents invoking a callable expression with a
     * list of argument expressions.
     *
     * Example:
     * @code
     * callee(arg, ...)
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class CallAccessExpression : public ExpressionNode {
        Expression              _callee;
        std::vector<Expression> _args;

       public:
        /** @brief Creates a function call expression. */
        CallAccessExpr(Expression callee, std::vector<Expression>&& args) :
            _callee(std::move(callee)), _args(args) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::CallAccess;
        }

        /** @brief Returns the callable expression. */
        Expression get_callee() const {
            return this->_callee;
        }

        /** @brief Returns the number of arguments. */
        std::size_t get_argc() const {
            return this->_args.size();
        }

        /** @brief Returns the argument at the specified position. */
        Expression get_arg_at(std::size_t pos) {
            return this->_args[pos];
        }

        /** @brief Returns an iterator to the first argument. */
        std::vector<Expression>::iterator arg_begin() {
            return this->_args.begin();
        }

        /** @brief Returns an iterator past the last argument. */
        std::vector<Expression>::iterator arg_end() {
            return this->_args.end();
        }
    };
}  // namespace Z::Zaban::AST::Expressions

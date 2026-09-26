#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST {
    /** @brief Represents a function expression.
     *
     * FunctionExpression represents an anonymous function definition, including
     * its parameters, optional return type, and optional function body.
     *
     * Function expressions can be used as values and passed or assigned like
     * other expressions.
     */
    template<typename OffsetType = std::size_t>
    class FunctionExpressionNode : public ExpressionNode {
        const std::vector<Parameter> _params;
        const Annotation             _return_type = nullptr;
        const Statement              _body        = nullptr;

       public:
        /** @brief Creates an empty function expression. */
        FunctionExpressionNode() : _params() {
        }

        /** @brief Creates a function expression with parameters. */
        FunctionExpressionNode(std::vector<Parameter>&& args) :
            _params(std::move(args)) {
        }

        /** @brief Creates a function expression with parameters and return
         * type. */
        FunctionExpressionNode(std::vector<Parameter>&& args,
                               Annotation               return_type) :
            _params(std::move(args)), _return_type(return_type) {
        }

        /** @brief Creates a complete function expression. */
        FunctionExpressionNode(std::vector<Parameter>&& args,
                               Annotation return_type, Statement&& body) :
            _params(std::move(args)), _return_type(return_type),
            _body(std::move(body)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Function;
        }

        /** @brief Returns the number of function parameters. */
        std::size_t get_argc() const {
            return this->_params.size();
        }

        /** @brief Returns the parameter at the specified position. */
        Parameter get_arg_at(std::size_t pos) {
            return this->_params[pos];
        }

        /** @brief Returns an iterator to the first parameter. */
        std::vector<Parameter>::const_iterator arg_begin() {
            return this->_params.begin();
        }

        /** @brief Returns an iterator past the last parameter. */
        std::vector<Parameter>::const_iterator arg_end() {
            return this->_params.end();
        }

        /** @brief Returns the function return type annotation, if available. */
        Annotation get_return_type() {
            return this->_return_type;
        }

        /** @brief Returns the function body, if available. */
        Statement get_body() {
            return this->_body;
        }
    };

    template<typename OffsetType = std::size_t>
    using FunctionExpression = std::shared_ptr<FunctionExpressionNode>;
}  // namespace Z::Zaban::AST

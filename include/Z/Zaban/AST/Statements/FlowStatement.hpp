#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST::Statements {
    /** @brief Identifies a control-flow transfer operation.
     *
     * A FlowKind represents statements that alter the normal execution flow of
     * a function or loop. These operations are handled during semantic analysis
     * and later lowered into control-flow graph operations.
     */
    enum class FlowKind {
        /** @brief Continue the current loop iteration.
         *
         * Transfers execution to the next iteration point of the nearest
         * enclosing loop.
         */
        Continue,
        /** @brief Exit the current loop.
         *
         * Transfers execution outside the nearest enclosing loop.
         */
        Break,
        /** @brief Jump to a labeled statement.
         *
         * Performs an explicit control-flow transfer to a target label.
         */
        Goto,
        /** @brief Return from the current function.
         *
         * Optionally transfers a return value to the caller depending on the
         * function return type.
         */
        Return,
    };

    /** @brief Represents a control-flow statement.
     *
     * FlowStatement represents statements that alter normal execution flow,
     * such as return, break, continue, and goto operations.
     *
     * A flow statement may optionally contain a target label or return
     * expression.
     */
    template<typename OffsetType = std::size_t>
    class FlowStatement : public StatementNode {
        const FlowKind                   _type;
        const std::optional<std::string> _jmplabel;
        const Expression                 _return_expr;

       public:
        /** @brief Creates a flow statement without additional data. */
        FlowStatement(FlowKind type) :
            _type(type), _jmplabel(std::nullopt), _return_expr(nullptr) {
        }

        /** @brief Creates a flow statement targeting a label. */
        FlowStatement(FlowKind type, std::string label) :
            _type(type), _jmplabel(label), _return_expr(nullptr) {
        }

        /** @brief Creates a return flow statement with an expression. */
        FlowStatement(FlowKind type, Expression return_expr) :
            _type(type), _jmplabel(std::nullopt),
            _return_expr(std::move(return_expr)) {
        }

        StatementKind statement_kind() const override {
            return StatementKind::Flow;
        }

        /** @brief Returns the flow operation kind. */
        FlowKind get_type() const {
            return this->_type;
        }

        /** @brief Returns whether this statement has a jump label. */
        bool has_label() const {
            switch (this->_type) {
                case FlowKind::Return:
                    return false;
                default:
                    return this->_jmplabel.has_value();
            }
        }

        /** @brief Returns the jump target label. */
        std::string get_label() const {
            return this->_jmplabel.value();
        }

        /** @brief Returns whether this statement contains a return expression.
         */
        bool has_return_expr() const {
            if (this->_type == FlowKind::Return &&
                this->_return_expr != nullptr) {
                return true;
            }
            return false;
        }

        /** @brief Returns the associated expression, if available. */
        Expression get_expr() const {
            return this->_return_expr;
        }
    };
}  // namespace Z::Zaban::AST::Statements

#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    /** @brief Represents a loop expression.
     *
     * LoopExpression represents a conditional loop construct containing
     * optional header expressions, condition lines, and an optional default
     * clause.
     *
     * The header expressions provide values used during loop evaluation, while
     * condition lines define the execution branches for each iteration.
     */
    template<typename OffsetType = std::size_t>
    class LoopExpressionNode : public ExpressionNode {
       private:
        std::vector<Expression>    _header;
        std::vector<ConditionLine> _lines;

        // Optional default clause.
        Statement _default = nullptr;

       public:
        /** @brief Creates a loop with condition lines only. */
        LoopExpressionNode(std::vector<ConditionLine>&& lines) :
            _header(), _lines(std::move(lines)) {
        }

        /** @brief Creates a loop with header expressions and condition lines.
         */
        LoopExpressionNode(std::vector<Expression>&&    headers,
                           std::vector<ConditionLine>&& lines) :
            _header(std::move(headers)), _lines(std::move(lines)) {
        }

        /** @brief Creates a loop with headers, condition lines, and a default
         * clause. */
        LoopExpressionNode(std::vector<Expression>&&    headers,
                           std::vector<ConditionLine>&& lines,
                           Statement&&                  defclause) :
            _header(std::move(headers)), _lines(std::move(lines)),
            _default(std::move(defclause)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Loop;
        }

        /** @brief Returns the number of header expressions. */
        const std::size_t get_headerc() const {
            return this->_header.size();
        }

        /** @brief Returns the number of condition lines. */
        const std::size_t get_condlc() const {
            return this->_lines.size();
        }

        /** @brief Returns a header expression by index. */
        Expression get_header_at(std::size_t pos) const {
            return this->_header[pos];
        }

        /** @brief Returns a condition line by index. */
        ConditionLine get_line_at(std::size_t pos) const {
            return this->_lines[pos];
        }

        /** @brief Returns the default clause, if present. */
        Statement get_default() const {
            return this->_default;
        }
    };

    template<typename OffsetType = std::size_t>
    using LoopExpression = std::shared_ptr<LoopExpressionNode>;
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

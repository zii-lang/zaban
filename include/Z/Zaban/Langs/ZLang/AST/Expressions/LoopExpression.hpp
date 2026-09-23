#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class LoopExpression : public ExpressionNode {
       private:
        std::vector<Expression>    _header;
        std::vector<ConditionLine> _lines;

        // Optional default clause.
        Statement _default = nullptr;

       public:
        /** @brief Creates a loop with condition lines only. */
        LoopExpr(std::vector<ConditionLine>&& lines) :
            _header(), _lines(std::move(lines)) {
        }

        /** @brief Creates a loop with header expressions and condition lines.
         */
        LoopExpr(std::vector<Expression>&&    headers,
                 std::vector<ConditionLine>&& lines) :
            _header(std::move(headers)), _lines(std::move(lines)) {
        }

        /** @brief Creates a loop with headers, condition lines, and a default
         * clause. */
        LoopExpr(std::vector<Expression>&&    headers,
                 std::vector<ConditionLine>&& lines, Statement&& defclause) :
            _header(std::move(headers)), _lines(std::move(lines)),
            _default(std::move(defclause)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind kind() const override {
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
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

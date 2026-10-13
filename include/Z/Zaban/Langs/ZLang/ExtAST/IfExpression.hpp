#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/ConditionLine.hpp>

namespace Z::Zaban::Langs::ZLang::ExtAST {
    /** @brief Represents a conditional expression.
     *
     * IfExpression represents a conditional construct containing optional
     * header expressions, condition lines, and an optional default clause.
     *
     * Header expressions provide values used by condition lines, while each
     * condition line defines a branch that is evaluated against those values.
     * The default clause is executed when no condition line matches.
     */
    template<typename OffsetType = std::size_t>
    class IfExpression : public AST::ExpressionNode<OffsetType> {
        using Expression     = AST::Expression<OffsetType>;
        using ExpressionKind = AST::ExpressionKind;
        using Statement      = AST::Statement<OffsetType>;
        using ConditionLine  = ConditionLine<OffsetType>;

        std::vector<Expression>    _header;
        std::vector<ConditionLine> _lines;

        // Optional default clause.
        Statement _default = nullptr;

       public:
        /** @brief Creates a conditional expression with condition lines only.
         */
        IfExpression(std::vector<ConditionLine>&& lines) :
            _header(), _lines(std::move(lines)) {
        }

        /** @brief Creates a conditional expression with headers and condition
         * lines. */
        IfExpression(std::vector<Expression>&&    headers,
                     std::vector<ConditionLine>&& lines) :
            _header(std::move(headers)), _lines(std::move(lines)) {
        }

        /** @brief Creates a conditional expression with a default clause. */
        IfExpression(std::vector<Expression>&&    headers,
                     std::vector<ConditionLine>&& lines,
                     Statement&&                  defclause) :
            _header(std::move(headers)), _lines(std::move(lines)),
            _default(std::move(defclause)) {
        }

        /** @brief Returns the expression kind. */
        ExpressionKind expr_kind() const override {
            return ExpressionKind::Conditional;
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

        Expression as_ptr() {
            return std::make_shared<IfExpression<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST

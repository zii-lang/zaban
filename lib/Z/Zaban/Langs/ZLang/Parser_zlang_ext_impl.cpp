#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/ExtAST/HeaderCondition.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <variant>

namespace Z::Zaban::Langs::ZLang {

    ExtAST::HeaderCondition<ZOffsetType> ZParser::parse_header_condition()
        const {
        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: somehow report error.
            return nullptr;
        }

        switch (token->kind) {
            case ZTokenKind::DQmark:
            case ZTokenKind::QExclam:
            case ZTokenKind::DExclam:
            case ZTokenKind::ColonColon: {
                ExtAST::HeaderConditionNode<ZOffsetType> condition({});
                condition.set_location(token->range);
                return condition.as_ptr();
            }
            default:
                break;
        }

        auto start_location                               = token->range.begin;
        ExtAST::HeaderConditions<ZOffsetType> expressions = {};
        auto expression = this->parse_expression();

        if (!expression) {
            // TODO: if we have reached here we need first expression.
            return nullptr;
        }
        expressions.emplace_back(std::move(expression));

        while (this->m_stream.match(ZTokenKind::Comma)) {
            expression = this->parse_expression();
            if (!expression) {
                // TODO: expression null
                return nullptr;
            }
            expressions.emplace_back(std::move(expression));
        }

        ExtAST::HeaderConditionNode<ZOffsetType> condition(expressions);
        condition.set_location(OffsetRange<ZOffsetType>(
            start_location, expression->location().end));

        return condition.as_ptr();
    }

    MaybeConditionLine ZParser::parse_condition_line() const {
        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report unexpected eof.
            return std::monostate{};
        }
        ExtAST::ConditionLineOperator op;
        switch (token->kind) {
            case ZTokenKind::DQmark:
                op = ExtAST::ConditionLineOperator::Chain;
                break;
            case ZTokenKind::QExclam:
                op = ExtAST::ConditionLineOperator::Serial;
                break;
            case ZTokenKind::DExclam:
                op = ExtAST::ConditionLineOperator::Parallel;
                break;
            default:
                // TODO: maybe report error.
                return nullptr;
        }

        this->m_stream.advance();  // consume operator.

        token = this->m_stream.peek();
        if (!token) {
            // TODO: report error
            return nullptr;
        }

        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_if() const {
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang

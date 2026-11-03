#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expressions/ConditionalExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/WhenClause.hpp>

namespace Z::Zaban::AST::ZLang::Expressions {
    template<typename OffsetType = std::size_t>
    class LoopExpressionNode : public ConditionalExpressionNode<OffsetType> {
       private:
       public:
        LoopExpressionNode(AST::Expression<OffsetType> sh_expression,
                           ElseClause<OffsetType>      else_clause) {
            this->m_sh_expression = sh_expression;
            this->m_else          = else_clause;
        };

        LoopExpressionNode(HeaderConditions<OffsetType>        headers,
                           std::vector<WhenClause<OffsetType>> whens,
                           ElseClause<OffsetType>              else_clause) {
            this->m_headers = std::move(headers);
            this->m_whens   = std::move(whens);
            this->m_else    = else_clause;
        };

        ConditionalExpressionKind condition_kind() override {
            return ConditionalExpressionKind::If;
        };

        AST::Expression<OffsetType> as_ptr() {
            return std::make_shared<LoopExpressionNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::ZLang::Expressions

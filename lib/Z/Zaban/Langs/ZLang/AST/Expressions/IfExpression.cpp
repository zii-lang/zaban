#include <Z/Zaban/Langs/ZLang/AST/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expressions/IfExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/WhenClause.hpp>

namespace Z::Zaban::AST::ZLang::Expressions {
    template<typename OffsetType>
    IfExpressionNode<OffsetType>::IfExpressionNode(
        Zaban::AST::Expression<OffsetType> sh_expression,
        ElseClause<OffsetType>             else_clause) {
        this->m_sh_expression = sh_expression;
        this->m_else          = else_clause;
    };

    template<typename OffsetType>
    IfExpressionNode<OffsetType>::IfExpressionNode(
        std::vector<WhenClause<OffsetType>> whens,
        ElseClause<OffsetType>              else_clause) {
        this->m_whens = std::move(whens);
        this->m_else  = else_clause;
    }
}  // namespace Z::Zaban::AST::ZLang::Expressions

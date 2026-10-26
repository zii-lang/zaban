#include <Z/Zaban/Langs/ZLang/AST/Expressions/IfExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/WhenClause.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType>
    IfExpressionNode<OffsetType>::IfExpressionNode(
        Zaban::AST::Expression<OffsetType> sh_expression,
        Extension::ElseClause<OffsetType>  else_clause) {
        this->m_sh_expression = sh_expression;
        this->m_else          = else_clause;
    };

    template<typename OffsetType>
    IfExpressionNode<OffsetType>::IfExpressionNode(
        std::vector<Extension::WhenClause<OffsetType>> whens,
        Extension::ElseClause<OffsetType>              else_clause) {
        this->m_whens = std::move(whens);
        this->m_else  = else_clause;
    }
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

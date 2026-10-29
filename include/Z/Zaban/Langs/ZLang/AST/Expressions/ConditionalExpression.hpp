#pragma once

#include <Z/Zaban/Langs/ZLang/AST/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/WhenClause.hpp>
#include <vector>

namespace Z::Zaban::AST::ZLang::Expressions {
    enum class ConditionalExpressionKind {
        Invalid,
        If,
        Loop,
    };

    template<typename OffsetType = std::size_t>
    class ConditionalExpressionNode : public ExpressionNode<OffsetType> {
       protected:
        Expression<OffsetType>              m_sh_expression;
        std::vector<WhenClause<OffsetType>> m_whens;
        ElseClause<OffsetType>              m_else;
        ConditionalExpressionKind m_kind = ConditionalExpressionKind::Invalid;

       public:
        ExpressionKind ex_expr_kind() const override;
        virtual ConditionalExpressionKind condition_kind() = 0;
    };
}  // namespace Z::Zaban::AST::ZLang::Expressions

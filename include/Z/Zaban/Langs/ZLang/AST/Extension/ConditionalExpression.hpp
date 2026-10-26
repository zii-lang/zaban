#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/WhenClause.hpp>
#include <vector>

namespace Z::Zaban::Langs::ZLang::AST::Extension {
    enum class ConditionalExpressionKind {
        Invalid,
        If,
        Loop,
    };

    template<typename OffsetType = std::size_t>
    class ConditionalExpressionNode
        : public Zaban::AST::ExpressionNode<OffsetType> {
       protected:
        Zaban::AST::Expression<OffsetType>  m_sh_expression;
        std::vector<WhenClause<OffsetType>> m_whens;
        ElseClause<OffsetType>              m_else;
        ConditionalExpressionKind m_kind = ConditionalExpressionKind::Invalid;

       public:
        Zaban::AST::ExpressionKind expr_kind() override {
            return Zaban::AST::ExpressionKind::Conditional;
        }

        virtual ConditionalExpressionKind condition_kind() = 0;
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Extension

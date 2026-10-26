#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/ConditionalExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/WhenClause.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class LoopExpressionNode
        : public Extension::ConditionalExpressionNode<OffsetType> {
       public:
        LoopExpressionNode();
        LoopExpressionNode(Zaban::AST::Expression<OffsetType>,
                           Extension::ElseClause<OffsetType>);
        LoopExpressionNode(Extension::WhenClause<OffsetType>,
                           Extension::ElseClause<OffsetType>);
        Extension::ConditionalExpressionKind condition_kind() override {
            return Extension::ConditionalExpressionKind::Loop;
        };
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expressions/ConditionalExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/WhenClause.hpp>

namespace Z::Zaban::AST::ZLang::Expressions {
    template<typename OffsetType = std::size_t>
    class IfExpressionNode : public ConditionalExpressionNode<OffsetType> {
       private:
       public:
        IfExpressionNode(AST::Expression<OffsetType>, ElseClause<OffsetType>);
        IfExpressionNode(std::vector<WhenClause<OffsetType>>,
                         ElseClause<OffsetType>);
        ConditionalExpressionKind condition_kind() override {
            return ConditionalExpressionKind::If;
        };

        AST::Expression<OffsetType> as_ptr() {
            return std::make_shared<IfExpressionNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::ZLang::Expressions

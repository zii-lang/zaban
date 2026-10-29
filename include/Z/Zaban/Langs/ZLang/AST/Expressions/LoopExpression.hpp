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
        LoopExpressionNode(AST::Expression<OffsetType>, ElseClause<OffsetType>);
        LoopExpressionNode(std::vector<WhenClause<OffsetType>>,
                           ElseClause<OffsetType>);
        ConditionalExpressionKind condition_kind() override {
            return ConditionalExpressionKind::If;
        };

        AST::Expression<OffsetType> as_ptr() {
            return std::make_shared<LoopExpressionNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::ZLang::Expressions

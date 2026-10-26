#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/ConditionalExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/ElseClause.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/WhenClause.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class IfExpressionNode
        : public Extension::ConditionalExpressionNode<OffsetType> {
       private:
       public:
        IfExpressionNode(Zaban::AST::Expression<OffsetType>,
                         Extension::ElseClause<OffsetType>);
        IfExpressionNode(std::vector<Extension::WhenClause<OffsetType>>,
                         Extension::ElseClause<OffsetType>);
        Extension::ConditionalExpressionKind condition_kind() override {
            return Extension::ConditionalExpressionKind::If;
        };

        Zaban::AST::Expression<OffsetType> as_ptr() {
            return std::make_shared<IfExpressionNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

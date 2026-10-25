#pragma once

#include <Z/Zaban/Langs/ZLang/AST/Extension/ConditionalExpression.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class IfExpressionNode
        : public AST::Extension::ConditionalExpressionNode<OffsetType> {
       public:
        IfExpressionNode();
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

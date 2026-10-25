#pragma once

#include <Z/Zaban/Langs/ZLang/AST/Extension/ConditionalExpression.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class LoopExpressionNode
        : public AST::Extension::ConditionalExpressionNode<OffsetType> {
       public:
        LoopExpressionNode();
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

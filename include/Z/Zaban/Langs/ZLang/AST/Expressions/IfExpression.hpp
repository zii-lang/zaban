#pragma once

#include <Z/Zaban/Langs/ZLang/AST/Abstract/ConditionalExpression.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class IfExpressionNode
        : public AST::Abstract::ConditionalExpressionNode<OffsetType> {
       public:
        IfExpressionNode<OffsetType>();
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

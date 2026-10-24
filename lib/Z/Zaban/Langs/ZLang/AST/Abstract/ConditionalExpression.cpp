#include <Z/Zaban/Langs/ZLang/AST/Abstract/ConditionalExpression.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Abstract {
    template<typename OffsetType>
    Zaban::AST::ExpressionKind
    ConditionalExpressionNode<OffsetType>::expr_kind() {
        return Zaban::AST::ExpressionKind::Conditional;
    }

    template<typename OffsetType>
    ConditionalExpressionKind
    ConditionalExpressionNode<OffsetType>::condition_kind() {
        return this->m_kind;
    }
}  // namespace Z::Zaban::Langs::ZLang::AST::Abstract

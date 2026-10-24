#include <Z/Zaban/Langs/ZLang/AST/Abstract/ConditionalExpression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Expressions/IfExpression.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Expressions {
    template<typename OffsetType>
    IfExpressionNode<OffsetType>::IfExpressionNode() {
        this->m_kind = Abstract::ConditionalExpressionKind::If;
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Expressions

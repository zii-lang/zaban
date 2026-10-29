#include <Z/Zaban/Langs/ZLang/AST/Expressions/ConditionalExpression.hpp>

namespace Z::Zaban::AST::ZLang::Expressions {
    template<typename OffsetType>
    ExpressionKind ConditionalExpressionNode<OffsetType>::ex_expr_kind() const {
        return ExpressionKind::Conditional;
    }
}  // namespace Z::Zaban::AST::ZLang::Expressions

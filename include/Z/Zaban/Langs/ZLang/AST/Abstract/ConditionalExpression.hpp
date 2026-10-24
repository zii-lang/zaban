#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Abstract {
    enum class ConditionalExpressionKind {
        Invalid,
        If,
        Loop,
    };

    template<typename OffsetType = std::size_t>
    class ConditionalExpressionNode
        : public Zaban::AST::ExpressionNode<OffsetType> {
       protected:
        ConditionalExpressionKind m_kind = ConditionalExpressionKind::Invalid;

       public:
        Zaban::AST::ExpressionKind expr_kind() override;
		ConditionalExpressionKind condition_kind();
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Abstract

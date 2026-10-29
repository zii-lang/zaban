#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::ZLang {
    enum class ExpressionKind {
        Invalid,
        Conditional,
    };

    template<typename OffsetType = std::size_t>
    class ExpressionNode : public AST::ExpressionNode<OffsetType> {
       public:
        AST::ExpressionKind expr_kind() const override {
            return AST::ExpressionKind::Extension;
        }
        virtual ExpressionKind ex_expr_kind() const = 0;
    };
}  // namespace Z::Zaban::AST::ZLang

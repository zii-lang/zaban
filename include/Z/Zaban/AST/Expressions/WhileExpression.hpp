#pragma once

#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Expressions {
    template<typename OffsetType = std::size_t>
    class WhileExpression : public ExpressionNode {};
}  // namespace Z::Zaban::AST::Expressions

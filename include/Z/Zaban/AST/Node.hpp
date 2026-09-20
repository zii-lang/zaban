#pragma once

namespace Z::Zaban::AST {
    enum class NodeKind {
        Expression,
        Statement,
    };

    class Node {
       public:
        virtual const NodeKind node_kind() const = 0;
    };
}  // namespace Z::Zaban::AST

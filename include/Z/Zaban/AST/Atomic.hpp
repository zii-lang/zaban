#pragma once

namespace Z::Zaban::AST {
    enum class AtomicKind {
        Literal,
        Identifier,
    };

    template<typename OffsetType>
    class AtomicNode : public Node {
       public:
        /** @brief Returns node type if required. */
        const NodeKind node_kind() const override {
            return NodeKind::Atomic;
        }

        virtual AtomicKind get_atomic_kind() const = 0;
    };
}  // namespace Z::Zaban::AST

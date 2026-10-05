#pragma once

#include <Z/Zaban/AST/Node.hpp>

namespace Z::Zaban::AST {
    enum class AtomicKind {
        Literal,
        Identifier,
    };

    template<typename OffsetType = std::size_t>
    class AtomicNode : public Node<OffsetType> {
       public:
        /** @brief Returns node type if required. */
        const NodeKind node_kind() const override {
            return NodeKind::Atomic;
        }

        virtual AtomicKind get_atomic_kind() const = 0;
    };

    template<typename OffsetType = std::size_t>
    using Atomic = std::shared_ptr<AtomicNode<OffsetType>>;
}  // namespace Z::Zaban::AST

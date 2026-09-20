#pragma once

#include <Z/Zaban/SourcePosition.hpp>
#include <memory>

namespace Z::Zaban::AST {
    enum class NodeKind {
        Expression,
        Statement,
        Declaration,
    };

    template<typename OffsetType = std::size_t>
    class Node : public std::enable_shared_from_this<Node> {
       public:
        virtual ~Node() = default;

        virtual const NodeKind          node_kind() const = 0;
        virtual OffsetRange<OffsetType> location() const  = 0;

        /** @brief Casts this node to a concrete node type.
         *
         * Provides a convenient way to access derived classes while
         * preserving shared ownership semantics.
         *
         * @tparam T The target node type.
         *
         * @return A shared pointer to the requested node.
         */
        template<typename T>
        inline std::shared_ptr<T> cast() {
            return std::static_pointer_cast<T>(shared_from_this());
        }
    };
}  // namespace Z::Zaban::AST

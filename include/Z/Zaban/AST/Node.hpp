#pragma once

#include <Z/Zaban/SourcePosition.hpp>
#include <memory>

namespace Z::Zaban::AST {
    enum class NodeKind {
        Atomic,
        Annotation,
        Expression,
        Statement,
        Declaration,
        Parameter,
    };

    template<typename OffsetType = std::size_t>
    class Node : public std::enable_shared_from_this<Node<OffsetType>> {
       private:
        mutable OffsetRange<OffsetType> m_location =
            OffsetRange<OffsetType>(0, 0);

       public:
        virtual ~Node() = default;

        virtual const NodeKind node_kind() const = 0;

        OffsetRange<OffsetType> location() const {
            return this->m_location;
        };

        void set_location(OffsetRange<OffsetType> location) const {
            this->m_location = location;
        }

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
            return std::static_pointer_cast<T>(this->shared_from_this());
        }
    };
}  // namespace Z::Zaban::AST

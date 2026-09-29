#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <variant>
#include <vector>

namespace Z::Zaban::AST::Atomics {
    /** @brief Represents an identifier reference in the AST.
     *
     * IIdentifier stores the name of a referenced symbol. Identifiers are
     * resolved during semantic analysis and later associated with their
     * corresponding bindings.
     */
    template<typename OffsetType = std::size_t>
    class IdentifierNode : public AtomicNode<OffsetType> {
       private:
        OffsetRange<OffsetType> m_location;

       public:
        /** @brief Creates an identifier from it's position. */
        IdentifierNode(OffsetRange<OffsetType> location) :
            m_location(location) {
        }
        IdentifierNode(OffsetRange<OffsetType>&& location) :
            m_location(std::move(location)) {
        }

        /** @brief Returns the primary expression category. */
        AtomicKind get_atomic_kind() const override {
            return AtomicKind::Identifier;
        }

        OffsetRange<OffsetType> location() const override {
            return this->m_location;
        }
    };

    template<typename OffsetType = std::size_t>
    using Identifier = std::shared_ptr<IdentifierNode<OffsetType>>;
}  // namespace Z::Zaban::AST::Atomics

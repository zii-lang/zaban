#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <variant>
#include <vector>

namespace Z::Zaban::AST::Atomics {
    /** @brief Represents an identifier reference in the AST.
     *
     * Identifier stores the name of a referenced symbol. Identifiers are
     * resolved during semantic analysis and later associated with their
     * corresponding bindings.
     */
    template<typename OffsetType = std::size_t>
    class Identifier : public AtomicNode<OffsetType> {
       private:
        OffsetRange<OffsetType> m_location;

       public:
        /** @brief Creates an identifier from it's position. */
        Identifier(OffsetRange<OffsetType> location) : m_location(location) {
        }
        Identifier(OffsetRange<OffsetType>&& location) :
            m_location(std::move(location)) {
        }

        /** @brief Returns the primary expression category. */
        AtomicKind get_atomic_kind() const override {
            return AtomicKind::Identifier;
        }

        OffsetRange<OffsetType> location() const override {
            return this->m_location;
        }

        Atomic<OffsetType> get_ptr() {
            return std::make_shared<Identifier<OffsetType>>(this);
        }
    };
}  // namespace Z::Zaban::AST::Atomics

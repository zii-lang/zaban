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
    class IdentifierNode : public Atomic {
       private:
        // Referenced identifier name.
        std::string _name;

       public:
        /** @brief Creates an identifier with the given name. */
        IdentifierNode(std::string&& name) : _name(std::move(name)) {
        }

        /** @brief Returns the primary expression category. */
        AtomicKind get_atomic_kind() const override {
            return AtomicKind::Identifier;
        }

        /** @brief Returns the referenced identifier name. */
        std::string get_name() const {
            return this->_name;
        }
    };

    template<typename OffsetType = std::size_t>
    using Identifier = std::shared_ptr<IdentifierNode>;
}  // namespace Z::Zaban::AST::Atomics

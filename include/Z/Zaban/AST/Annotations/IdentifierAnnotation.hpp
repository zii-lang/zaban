#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a named type annotation.
     *
     * IdentifierAnnotation describes a type referenced by name. The referenced
     * type is resolved during semantic analysis.
     */
    template<typename OffsetType = std::size_t>
    class IdentifierAnnotation : public AnnotationNode {
       private:
        std::string id;

        // Lexical scope path where this identifier is referenced.
        ScopeSet _scope_set;

        // Binding resolved during semantic analysis.
        std::optional<BindingId> _binding = std::nullopt;

       public:
        /** @brief Creates an identifier annotation with the given name. */
        explicit IdentifierAnnotation(std::string i) : id(std::move(i)) {
        }

        /** @brief Returns the annotation category. */
        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Base;
        }

        /** @brief Returns the base annotation category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Identifier;
        }

        /** @brief Returns the referenced identifier name. */
        std::string get_id() const {
            return id;
        }

        /** @brief Returns the lexical scope path of this identifier. */
        ScopeSet& scope_set() {
            return _scope_set;
        }

        /** @brief Returns the lexical scope path of this identifier. */
        const ScopeSet& scope_set() const {
            return _scope_set;
        }

        /** @brief Assigns the resolved semantic binding. */
        void set_binding(BindingId id) {
            _binding = id;
        }

        /** @brief Returns the resolved binding, if available. */
        std::optional<BindingId> binding() const {
            return _binding;
        }
    };

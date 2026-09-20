#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Declaration.hpp>

namespace Z::Zaban::AST {
    /** @brief Represents a type declaration.
     *
     * TypeDeclaration introduces a named type into the current scope. The
     * declaration contains the type name, its associated annotation, and the
     * semantic binding assigned during name resolution.
     *
     * Example:
     * @code
     * type Name : i32;
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class TypeDeclaration : public DeclarationNode<OffsetType> {
       private:
        const std::string _name;
        const Annotation  _annotation;

        // Binding assigned during semantic analysis.
        // std::optional<BindingId> _binding = std::nullopt;

       public:
        TypeDeclaration(std::string name, Annotation annotation) :
            _name(name), _annotation(std::move(annotation)) {
        }

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::TypeDecl;
        }

        /** @brief Returns the declared type name. */
        const std::string get_name() const {
            return _name;
        }

        /** @brief Returns the type annotation. */
        const Annotation get_annotation() const {
            return _annotation;
        }

        // /** @brief Returns the resolved binding, if available. */
        // std::optional<BindingId> binding() const {
        //     return _binding;
        // }

        // /** @brief Assigns the semantic binding for this declaration. */
        // void set_binding(BindingId id) {
        //     _binding = id;
        // }
    };

}  // namespace Z::Zaban::AST

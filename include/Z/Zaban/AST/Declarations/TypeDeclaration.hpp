#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Declaration.hpp>

namespace Z::Zaban::AST::Declarations {
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
        // TODO: switch from string to Atomic, this is supposed to be Identifier
        // atomic and semantic pass checks if it is correct or not.
        const std::string _name;
        const Annotation  _annotation;

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
    };

}  // namespace Z::Zaban::AST::Declarations

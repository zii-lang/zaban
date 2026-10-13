#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Atomic.hpp>
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
        const Atomic<OffsetType>     m_id;
        const Annotation<OffsetType> m_annotation;

       public:
        TypeDeclaration(Atomic<OffsetType>     id,
                        Annotation<OffsetType> annotation) :
            m_id(id), m_annotation(annotation) {};

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::TypeDecl;
        }

        /** @brief Returns the declared type name. */
        const Atomic<OffsetType> get_id() const {
            return this->m_id;
        }

        /** @brief Returns the type annotation. */
        const Annotation<OffsetType> get_annotation() const {
            return this->m_annotation;
        }

        Declaration<OffsetType> as_ptr() {
            return std::make_shared<TypeDeclaration<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Declarations

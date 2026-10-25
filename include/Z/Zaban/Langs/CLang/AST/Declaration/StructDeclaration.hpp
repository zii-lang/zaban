#pragma once

#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/StructAnnotation.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace Z::Zaban::Langs::CLang::AST::Declaration {

    /** @brief Represents a standalone struct definition or forward declaration.
     *
     * The annotation records the tag and whether a definition body appeared.
     * The declaration's source range includes the terminating semicolon and
     * is assigned by the parser through set_location().
     */
    template<typename OffsetType = std::size_t>
    class StructDeclarationNode
        : public Zaban::AST::DeclarationNode<OffsetType> {
       private:
        const std::shared_ptr<Annotations::StructAnnotationNode<OffsetType>>
            _annot;

       public:
        /** @brief Creates a declaration with a non-null struct annotation. */
        explicit StructDeclarationNode(
            Annotations::StructAnnotation<OffsetType> annotation) :
            _annot(std::move(annotation)) {
            assert(_annot != nullptr);
        }

        /** @brief Returns the type declaration category. */
        const Zaban::AST::DeclarationKind kind() const override {
            return Zaban::AST::DeclarationKind::TypeDecl;
        }

        /** @brief Returns the struct definition or tag reference. */
        Annotations::StructAnnotation<OffsetType> get_annotation() const {
            return this->_annot;
        }

        Zaban::AST::Declaration<OffsetType> as_ptr() const {
            return std::make_shared<StructDeclarationNode<OffsetType>>(*this);
        }
    };

    template<typename OffsetType = std::size_t>
    using StructDeclaration =
        std::shared_ptr<StructDeclarationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Declaration

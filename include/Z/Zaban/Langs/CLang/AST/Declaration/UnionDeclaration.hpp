#pragma once

#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/UnionAnnotation.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace Z::Zaban::Langs::CLang::AST::Declaration {

    /** @brief Represents a standalone union definition or forward declaration.
     *
     * The annotation records the tag and whether a definition body appeared.
     * The declaration's source range includes the terminating semicolon and
     * is assigned by the parser through set_location().
     */
    template<typename OffsetType = std::size_t>
    class UnionDeclarationNode
        : public Zaban::AST::DeclarationNode<OffsetType> {
       private:
        const std::shared_ptr<Annotations::UnionAnnotationNode<OffsetType>>
            _annot;

       public:
        /** @brief Creates a declaration with a non-null union annotation. */
        explicit UnionDeclarationNode(
            Annotations::UnionAnnotation<OffsetType> annotation) :
            _annot(std::move(annotation)) {
            assert(_annot != nullptr);
        }

        /** @brief Returns the type declaration category. */
        const Zaban::AST::DeclarationKind kind() const override {
            return Zaban::AST::DeclarationKind::TypeDecl;
        }

        /** @brief Returns the union definition or tag reference. */
        Annotations::UnionAnnotation<OffsetType> get_annotation() const {
            return this->_annot;
        }

        Zaban::AST::Declaration<OffsetType> as_ptr() const {
            return std::make_shared<UnionDeclarationNode<OffsetType>>(*this);
        }
    };

    template<typename OffsetType = std::size_t>
    using UnionDeclaration = std::shared_ptr<UnionDeclarationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Declaration

#pragma once

#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/EnumAnnotation.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace Z::Zaban::Langs::CLang::AST::Declaration {

    /** @brief Represents a standalone enum definition or forward declaration.
     *
     * The annotation records the tag and whether a definition body appeared.
     * The declaration's source range includes the terminating semicolon and
     * is assigned by the parser through set_location().
     */
    template<typename OffsetType = std::size_t>
    class EnumDeclarationNode : public Zaban::AST::DeclarationNode<OffsetType> {
       private:
        const Annotations::EnumAnnotation<OffsetType> _annot;

       public:
        /** @brief Creates a declaration with a non-null enum annotation. */
        explicit EnumDeclarationNode(
            Annotations::EnumAnnotation<OffsetType> annotation) :
            _annot(std::move(annotation)) {
            assert(_annot != nullptr);
        }

        /** @brief Returns the type declaration category. */
        const Zaban::AST::DeclarationKind kind() const override {
            return Zaban::AST::DeclarationKind::TypeDecl;
        }

        /** @brief Returns the enum definition or tag reference. */
        Annotations::EnumAnnotation<OffsetType> get_annotation() const {
            return this->_annot;
        }

        Zaban::AST::Declaration<OffsetType> as_ptr() const {
            return std::make_shared<EnumDeclarationNode<OffsetType>>(*this);
        }
    };

    template<typename OffsetType = std::size_t>
    using EnumDeclaration = std::shared_ptr<EnumDeclarationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Declaration

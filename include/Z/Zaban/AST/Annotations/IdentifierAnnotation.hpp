#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <string>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a named type annotation.
     *
     * IdentifierAnnotation describes a type referenced by name. The referenced
     * type is resolved during semantic analysis.
     */
    template<typename OffsetType = std::size_t>
    class IdentifierAnnotation : public BaseAnnotationNode<OffsetType> {
       private:
        std::string id;

       public:
        /** @brief Creates an identifier annotation with the given name. */
        explicit IdentifierAnnotation(std::string i) : id(std::move(i)) {
        }

        /** @brief Returns the base annotation category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Identifier;
        }

        /** @brief Returns the referenced identifier name. */
        std::string get_id() const {
            return id;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

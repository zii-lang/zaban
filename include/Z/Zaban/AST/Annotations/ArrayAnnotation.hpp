#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {

    /** @brief Represents an array type annotation.
     *
     * Stores the element type and optional array size expression.
     */
    template<typename OffsetType = std::size_t>
    class ArrayAnnotation : public AnnotationNode {
        Annotation element;
        Expr       num_elements;

       public:
        explicit ArrayAnnotation(Annotation e, Expr num_elements) :
            element(std::move(e)), num_elements(num_elements) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Array;
        }

        /** @brief Returns the array element annotation. */
        Annotation get_element() const {
            return element;
        }

        /** @brief Returns the expression defining the number of elements. */
        Expr get_num_elements() const {
            return num_elements;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

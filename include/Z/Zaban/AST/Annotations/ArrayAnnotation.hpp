#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Annotations {

    /** @brief Represents an array type annotation.
     *
     * Stores the element type and optional array size expression.
     */
    template<typename OffsetType = std::size_t>
    class ArrayAnnotation : public AnnotationNode<OffsetType> {
        Annotation<OffsetType> element;
        Expression<OffsetType> num_elements;

       public:
        explicit ArrayAnnotation(Annotation<OffsetType> e,
                                 Expression<OffsetType> num_elements) :
            element(std::move(e)), num_elements(std::move(num_elements)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Array;
        }

        /** @brief Returns the array element annotation. */
        Annotation<OffsetType> get_element() const {
            return element;
        }

        /** @brief Returns the expression defining the number of elements. */
        Expression<OffsetType> get_num_elements() const {
            return num_elements;
        }

        Annotation<OffsetType> as_ptr() const {
            return std::make_shared<PointerAnnotation<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Annotations

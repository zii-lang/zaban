#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {

    /** @brief Represents a pointer type annotation.
     *
     * Wraps another annotation as the pointed-to type.
     */
    template<typename OffsetType = std::size_t>
    class PointerAnnotation : public AnnotationNode {
       private:
        Annotation pointee;

       public:
        explicit PointerAnnotation(Annotation p) : pointee(std::move(p)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Pointer;
        }

        /** @brief Returns the pointed-to annotation. */
        Annotation get_pointee() const {
            return pointee;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

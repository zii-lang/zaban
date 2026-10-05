#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {

    /** @brief Represents a pointer type annotation.
     *
     * Wraps another annotation as the pointed-to type.
     */
    template<typename OffsetType = std::size_t>
    class PointerAnnotation : public AnnotationNode<OffsetType> {
       private:
        Annotation<OffsetType> pointee;

       public:
        explicit PointerAnnotation(Annotation<OffsetType> p) :
            pointee(std::move(p)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Pointer;
        }

        /** @brief Returns the pointed-to annotation. */
        Annotation<OffsetType> get_pointee() const {
            return pointee;
        }

        Annotation<OffsetType> as_ptr() {
            return std::make_shared<PointerAnnotation<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Annotations

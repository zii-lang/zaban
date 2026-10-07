#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a variadic argument type annotation (`...`). */
    template<typename OffsetType = std::size_t>
    class VarargAnnotation : public AnnotationNode<OffsetType> {
       public:
        VarargAnnotation() {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Vararg;
        }

        Annotation<OffsetType> as_ptr() const {
            return std::make_shared<VarargAnnotation<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Annotations

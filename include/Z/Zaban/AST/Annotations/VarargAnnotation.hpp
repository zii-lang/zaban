#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a variadic argument type annotation (`...`). */
    template<typename OffsetType = std::size_t>
    class VarargAnnotation : public AnnotationNode {
       public:
        VarargAnnotation() {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Vararg;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

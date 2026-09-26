#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a struct type annotation with its declared fields. */
    template<typename OffsetType = std::size_t>
    class StructAnnotation : public AnnotationNode {
       private:
        std::vector<Parameter> fields;

       public:
        explicit StructAnnotation(std::vector<Parameter> f) :
            fields(std::move(f)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Base;
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Struct;
        }

        /** @brief Returns the struct fields. */
        const std::vector<Parameter>& get_fields() const {
            return fields;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

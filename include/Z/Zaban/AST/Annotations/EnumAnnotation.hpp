#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents an enum type annotation with its declared fields. */
    template<typename OffsetType = std::size_t>
    class EnumAnnotation : public AnnotationNode {
       private:
        std::vector<Parameter> fields;

       public:
        explicit EnumAnnotation(std::vector<Parameter> f) :
            fields(std::move(f)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Base;
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Enum;
        }

        /** @brief Returns the enum fields. */
        const std::vector<Parameter>& get_fields() const {
            return fields;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

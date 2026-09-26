#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a variant type annotation with its possible variants.
     */
    template<typename OffsetType = std::size_t>
    class VariantAnnotation : public AnnotationNode {
       private:
        std::vector<VariantField> variants;

       public:
        explicit VariantAnnotation(std::vector<VariantField> v) :
            variants(std::move(v)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Base;
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Variant;
        }

        /** @brief Returns the variant alternatives. */
        const std::vector<VariantField>& get_variants() const {
            return variants;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

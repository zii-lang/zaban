#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <vector>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a variant type annotation with its possible variants.
     */
    template<typename OffsetType = std::size_t>
    class VariantAnnotation : public BaseAnnotationNode<OffsetType> {
       private:
        std::vector<Parameter<OffsetType>> variants;

       public:
        explicit VariantAnnotation(std::vector<Parameter<OffsetType>> v) :
            variants(std::move(v)) {
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Variant;
        }

        /** @brief Returns the variant alternatives. */
        const std::vector<Parameter<OffsetType>>& get_variants() const {
            return variants;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

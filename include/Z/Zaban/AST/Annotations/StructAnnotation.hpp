#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <vector>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a struct type annotation with its declared fields. */
    template<typename OffsetType = std::size_t>
    class StructAnnotation : public BaseAnnotationNode<OffsetType> {
       private:
        std::vector<Parameter<OffsetType>> fields;

       public:
        explicit StructAnnotation(std::vector<Parameter<OffsetType>> f) :
            fields(std::move(f)) {
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Struct;
        }

        /** @brief Returns the struct fields. */
        const std::vector<Parameter<OffsetType>>& get_fields() const {
            return fields;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

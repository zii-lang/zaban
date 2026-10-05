#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <vector>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents an enum type annotation with its declared fields. */
    template<typename OffsetType = std::size_t>
    class EnumAnnotation : public BaseAnnotationNode<OffsetType> {
       private:
        std::vector<Parameter<OffsetType>> fields;

       public:
        explicit EnumAnnotation(std::vector<Parameter<OffsetType>> f) :
            fields(std::move(f)) {
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Enum;
        }

        /** @brief Returns the enum fields. */
        const std::vector<Parameter<OffsetType>>& get_fields() const {
            return fields;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

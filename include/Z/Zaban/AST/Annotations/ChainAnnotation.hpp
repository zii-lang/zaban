#pragma once

#include <Z/Zaban/AST/Annotation.hpp>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a chained type annotation.
     *
     * Used to represent composed types such as function signatures:
     *
     * i32 -> i32 -> void
     */
    template<typename OffsetType = std::size_t>
    class ChainAnnotation : public AnnotationNode {
        Annotation _from;
        Annotation _to;

       public:
        ChainAnnotation(Annotation from, Annotation to) :
            _from(std::move(from)), _to(std::move(to)) {
        }

        AnnotationKind get_annotation_kind() const override {
            return AnnotationKind::Chain;
        }

        /** @brief Returns the input side of the chain. */
        const Annotation& get_from() const {
            return _from;
        }

        /** @brief Returns the output side of the chain. */
        const Annotation& get_to() const {
            return _to;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <string>

namespace Z::Zaban::AST::Annotations {
    /** @brief Represents a primitive type annotation.
     *
     * PrimitiveAnnotation describes built-in language types such as integers,
     * floating-point types, booleans, and other fundamental types.
     */
    template<typename OffsetType = std::size_t>
    class PrimitiveAnnotation : public BaseAnnotationNode<OffsetType> {
       private:
        std::string name;

       public:
        /** @brief Creates a primitive annotation with the given type name. */
        explicit PrimitiveAnnotation(std::string n) : name(std::move(n)) {
        }

        /** @brief Returns the base type category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Primitive;
        }

        /** @brief Returns the primitive type name. */
        std::string get_name() const {
            return name;
        }
    };
}  // namespace Z::Zaban::AST::Annotations

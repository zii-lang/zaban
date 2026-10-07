#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Atomic.hpp>
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
        AST::Atomic<OffsetType> m_value;

       public:
        /** @brief Creates a primitive annotation with the given type name. */
        explicit PrimitiveAnnotation(AST::Atomic<OffsetType> value) :
            m_value(std::move(value)) {
            this->set_location(this->m_value->location());
        }

        /** @brief Returns the base type category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Primitive;
        }

        /** @brief Returns the primitive type value (it's either like i32 in
         * languages like rust or in Z its string literal). */
        std::string get_value() const {
            return m_value;
        }

        Annotation<OffsetType> as_ptr() const {
            return std::make_shared<PrimitiveAnnotation<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Annotations

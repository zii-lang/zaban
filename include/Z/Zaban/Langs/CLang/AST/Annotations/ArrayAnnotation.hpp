#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/QualifierAnnotation.hpp>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Annotations {
    /** @brief An array parameter written with C99 parameter-only forms.
     *
     * Only used when the brackets hold qualifiers, `static`, or `*`. Plain
     * arrays use the shared Zaban::AST::Annotations::ArrayAnnotation.
     *
     * Example:
     * @code
     * void f(int a[const]);         // a is `int *const`
     * void f(int a[static 3]);      // caller passes at least 3 elements
     * void f(int a[*]);             // VLA of unspecified size
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class CArrayAnnotationNode : public Zaban::AST::AnnotationNode<OffsetType> {
       private:
        // what the array holds
        Zaban::AST::Annotation<OffsetType> _element;
        // null for [] and [*]
        Zaban::AST::Expression<OffsetType> _size;
        // list of type qualifiers [const ...]
        std::vector<TypeQualifier<OffsetType>> _qualifiers;
        // [static N]
        bool _is_static;
        // [*]
        bool _is_unspecified;

       public:
        CArrayAnnotationNode(Zaban::AST::Annotation<OffsetType> underlying,
                             Zaban::AST::Expression<OffsetType> sz,
                             std::vector<TypeQualifier<OffsetType>>&& quals,
                             bool is_static, bool is_unspecified) :
            _element(std::move(underlying)), _size(std::move(sz)),
            _qualifiers(std::move(quals)), _is_static(is_static),
            _is_unspecified(is_unspecified) {
        }

        /** @brief Returns the annotation kind. */
        Zaban::AST::AnnotationKind get_annotation_kind() const override {
            return Zaban::AST::AnnotationKind::Array;
        }

        /** @brief Returns the qualifier list */
        const std::vector<TypeQualifier<OffsetType>>& get_qualifiers() const {
            return this->_qualifiers;
        }
        /** @brief Returns the underlying type annotation */
        const Zaban::AST::Annotation<OffsetType>& get_element() const {
            return this->_element;
        }
        /** @brief Returns true for [static N]*/
        bool is_static() const {
            return this->_is_static;
        }
        /** @brief Returns true for [*]*/
        bool is_unspecified() const {
            return this->_is_unspecified;
        }

        /** @brief Returns array's size*/
        Zaban::AST::Expression<OffsetType> size() const {
            return this->_size;
        }
    };

    template<typename OffsetType = std::size_t>
    using CArrayAnnotation = std::shared_ptr<CArrayAnnotationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Annotations

#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Annotations {
    /** @brief A single type qualifier keyword and where it appeared. */
    template<typename OffsetType = std::size_t>
    struct TypeQualifier {
        CLang::TokenKind        kind;
        OffsetRange<OffsetType> range;
    };

    /** @brief Represents a C qualifier spelled with one or more keywords.
     *
     * C allows a type to be written with qualifiers
     * in any order. The parser only collects them, in
     * source order;
     * Example:
     * @code
     * const unsigned long x;
     * unsigned const long x;
     * struct S const *p;
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class QualifierAnnotationNode
        : public Zaban::AST::AnnotationNode<OffsetType> {
        Zaban::AST::Annotation<OffsetType>     _underlying;
        std::vector<TypeQualifier<OffsetType>> _qualifiers;

       public:
        /** @brief Creates a qualified annotation from its qualifiers. */
        QualifierAnnotationNode(
            Zaban::AST::Annotation<OffsetType>       underlying,
            std::vector<TypeQualifier<OffsetType>>&& qualifiers) :
            _underlying(std::move(underlying)),
            _qualifiers(std::move(qualifiers)) {
        }

        /** @brief Returns the annotation kind. */
        Zaban::AST::AnnotationKind get_annotation_kind() const override {
            return Zaban::AST::AnnotationKind::Qualified;
        }

        /** @brief Returns the qualifier list */
        const std::vector<TypeQualifier<OffsetType>>& get_qualifiers() const {
            return this->_qualifiers;
        }
        /** @brief Returns the underlying type annotation */
        const Zaban::AST::Annotation<OffsetType>& get_underlying() const {
            return this->_underlying;
        }
    };

    template<typename OffsetType = std::size_t>
    using QualifierAnnotation =
        std::shared_ptr<QualifierAnnotationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Annotations

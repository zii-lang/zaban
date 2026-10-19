#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <memory>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Annotations {
    /** @brief A single type specifier keyword and where it appeared. */
    template<typename OffsetType = std::size_t>
    struct TypeSpecifier {
        CLang::TokenKind        kind;
        OffsetRange<OffsetType> range;
    };

    /** @brief Represents a C primitive type spelled with one or more keywords.
     *
     * C allows a primitive type to be written as several specifier keywords
     * in any order, some of them repeated. The parser only collects them, in
     * source order; checking that the combination is valid (C11 §6.7.2p2) and
     * resolving it to a single type is left to semantic analysis.
     *
     * Example:
     * @code
     * unsigned long long int x;
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class PrimitiveAnnotationNode
        : public Zaban::AST::BaseAnnotationNode<OffsetType> {
        std::vector<TypeSpecifier<OffsetType>> _specifiers;

       public:
        /** @brief Creates a primitive annotation from its specifiers. */
        PrimitiveAnnotationNode(
            std::vector<TypeSpecifier<OffsetType>>&& specifiers) :
            _specifiers(std::move(specifiers)) {
        }

        /** @brief Returns the base annotation category. */
        Zaban::AST::BaseAnnotationKind get_base_kind() const override {
            return Zaban::AST::BaseAnnotationKind::CPrimitive;
        }

        /** @brief Returns the number of specifiers. */
        std::size_t get_specifierc() const {
            return this->_specifiers.size();
        }

        /** @brief Returns the specifier at the specified position. */
        const TypeSpecifier<OffsetType>& get_specifier_at(
            std::size_t pos) const {
            return this->_specifiers[pos];
        }

        /** @brief Returns an iterator to the first specifier. */
        typename std::vector<TypeSpecifier<OffsetType>>::const_iterator
        specifier_begin() const {
            return this->_specifiers.begin();
        }

        /** @brief Returns an iterator past the last specifier. */
        typename std::vector<TypeSpecifier<OffsetType>>::const_iterator
        specifier_end() const {
            return this->_specifiers.end();
        }
    };

    template<typename OffsetType = std::size_t>
    using PrimitiveAnnotation =
        std::shared_ptr<PrimitiveAnnotationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Annotations

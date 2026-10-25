#pragma once

#include <Z/Zaban/AST/Annotations/StructAnnotation.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Annotations {
    /** @brief Represents a C struct definition or a reference to a struct tag.
     *
     * Definitions may be unnamed. References always have a tag and store no
     * fields; resolving the tag to its definition is left to semantic analysis.
     */
    template<typename OffsetType = std::size_t>
    class StructAnnotationNode
        : public Zaban::AST::Annotations::StructAnnotation<OffsetType> {
        const std::shared_ptr<Zaban::AST::Atomics::Identifier<OffsetType>> _tag;
        const bool _has_definition;

       public:
        /** @brief Creates a reference to a non-null struct tag. */
        explicit StructAnnotationNode(
            std::shared_ptr<Zaban::AST::Atomics::Identifier<OffsetType>> tag) :
            Zaban::AST::Annotations::StructAnnotation<OffsetType>({}),
            _tag(std::move(tag)), _has_definition(false) {
            assert(_tag != nullptr);
        }

        /** @brief Creates a definition with an optional tag and its fields. */
        StructAnnotationNode(
            std::shared_ptr<Zaban::AST::Atomics::Identifier<OffsetType>> tag,
            std::vector<Zaban::AST::Parameter<OffsetType>> fields) :
            Zaban::AST::Annotations::StructAnnotation<OffsetType>(
                std::move(fields)),
            _tag(std::move(tag)), _has_definition(true) {
        }

        /** @brief Returns the tag identifier, or nullptr for an unnamed type.
         */
        std::shared_ptr<Zaban::AST::Atomics::Identifier<OffsetType>> get_tag()
            const {
            return this->_tag;
        }

        /** @brief Returns whether a struct definition body appeared. */
        bool has_definition() const {
            return this->_has_definition;
        }

        /** @brief Copies this annotation while preserving its C metadata. */
        Zaban::AST::Annotation<OffsetType> as_ptr() const {
            return std::make_shared<StructAnnotationNode<OffsetType>>(*this);
        }
    };

    template<typename OffsetType = std::size_t>
    using StructAnnotation = std::shared_ptr<StructAnnotationNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Annotations

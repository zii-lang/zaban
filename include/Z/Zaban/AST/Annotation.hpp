#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <memory>

namespace Z::Zaban::AST {
    /** @brief Represents the kind of a type annotation node.
     *
     * Type annotations are composed as a hierarchy of annotation nodes. Each
     * AnnotationKind identifies the role of a node within a type expression,
     * allowing complex types to be represented by combining multiple
     * annotations.
     */
    enum class AnnotationKind {
        /// A base type annotation (primitive or user-defined type).
        Base,
        /// A pointer type annotation.
        Pointer,
        /// An array type annotation.
        Array,
        /// A chained annotation that combines multiple annotation nodes into a
        /// complete type expression.
        Chain,
        /// A variadic argument annotation (e.g. `...`).
        Vararg,
    };

    /** @brief Identifies the fundamental category of a type annotation.
     *
     * A BaseAnnotationKind describes the underlying type referenced by a type
     * annotation before any modifiers (such as pointers, references, arrays,
     * or qualifiers) are applied. It is used by the parser and semantic
     * analysis to distinguish between built-in types and user-defined
     * declarations.
     */
    enum class BaseAnnotationKind {
        /// A primitive type (e.g. int, bool, float).
        Primitive,
        /// A user-defined type referenced by its identifier.
        /// The actual declaration is resolved during semantic analysis.
        Identifier,
        /// An enumeration type.
        Enum,
        /// A structure type.
        Struct,
        /// A variant (sum/union) type.
        Variant,
    };

    /** @brief Base interface for all type annotation AST nodes.
     *
     * IAnnotation represents the common interface shared by all annotation
     * nodes in the AST. An annotation describes a type expression and can
     * represent different forms of types such as base types, pointers, arrays,
     * or chained type compositions.
     *
     * Concrete annotation classes derive from IAnnotation and provide the
     * structure and semantics for a specific AnnotationKind.
     *
     * The class uses shared ownership semantics through
     * std::enable_shared_from_this to allow annotation nodes to safely create
     * shared references to themselves.
     */
    template<typename OffsetType = std::size_t>
    class AnnotationNode : public Node<OffsetType> {
       public:
        /** @brief Virtual destructor.
         *
         * Ensures proper destruction of derived annotation nodes through a base
         * class pointer.
         */
        virtual ~AnnotationNode() = default;

        /** @brief Returns the kind of this annotation node.
         *
         * @return The AnnotationKind identifying the concrete annotation type.
         */
        virtual AnnotationKind get_annotation_kind() const = 0;

        const NodeKind node_kind() const override {
            return NodeKind::Annotation;
        }
    };

    /** @brief Shared reference to a type annotation node.
     *
     * Annotation represents a type expression in the Abstract Syntax Tree.
     * Type annotations describe the structure and composition of types,
     * including base types, pointers, arrays, and chained function signatures.
     * The underlying IAnnotation node is managed through shared ownership,
     * allowing annotation nodes to be referenced by multiple AST structures
     * while their lifetime is automatically managed.
     */
    template<typename OffsetType = std::size_t>
    using Annotation = std::shared_ptr<AnnotationNode>;
}  // namespace Z::Zaban::AST

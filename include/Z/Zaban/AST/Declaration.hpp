#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <memory>

namespace Z::Zaban::AST {
    /** @brief Identifies the kind of declaration node in the Abstract Syntax
     * Tree.
     *
     * A DeclarationKind represents the category of a declaration introduced in
     * source code. Declarations introduce named entities into a scope, such as
     * types or variables.
     */
    enum class DeclarationKind {
        /** @brief A type declaration.
         *
         * Introduces a user-defined type into the program, such as a structure,
         * variant, enum, or other named type.
         *
         * Example:
         * @code
         * type i32 = "i32";
         * @endcode
         */
        TypeDecl,
        /** @brief A variable binding declaration.
         *
         * Introduces a named value binding.
         *
         * Example:
         * @code
         * let value: i32 = 10;
         * @endcode
         */
        LetDecl,
    };

    /** @brief Base interface for declaration AST nodes.
     *
     * DeclarationNode represents entities that introduce names into a scope,
     * such as type declarations and variable declarations.
     */
    template<typename OffsetType = std::size_t>
    class DeclarationNode : public Node<OffsetType> {
       public:
        /** @brief Virtual destructor for derived declaration nodes. */
        virtual ~DeclarationNode() = default;

        /** @brief Returns the declaration kind. */
        virtual const DeclarationKind kind() const = 0;

        const NodeKind node_kind() const override {
            return NodeKind::Declaration;
        }
    };

    /** @brief Shared reference to a declaration AST node.
     *
     * Declaration provides shared ownership semantics for Declaration nodes in
     * the AST.
     */
    template<typename OffsetType = std::size_t>
    using Declaration = std::shared_ptr<DeclarationNode>;
}  // namespace Z::Zaban::AST

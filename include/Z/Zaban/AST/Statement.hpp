#pragma once

#include <Z/Zaban/AST/Node.hpp>

namespace Z::Zaban::AST {
    /** @brief Identifies the kind of statement node in the Abstract Syntax
     * Tree.
     *
     * A StatementKind represents the category of a statement. Statements
     * describe executable actions, declarations, metadata, or structural
     * elements that control how code is organized and evaluated.
     */
    enum class StatementKind {
        /** @brief An expression statement.
         *
         * Represents an expression evaluated for its side effects or resulting
         * value.
         */
        Expression,
        /** @brief A control-flow statement.
         *
         * Represents statements that alter execution flow.
         *
         * Examples:
         * @code
         * break;
         * continue;
         * return value;
         * @endcode
         */
        Flow,
        /** @brief A block statement.
         *
         * Represents a sequence of statements grouped into a lexical scope.
         *
         * Example:
         * @code
         * {
         *     let x: i32 = 10;
         * }
         * @endcode
         */
        Block,
        /** @brief A label statement.
         *
         * Represents a named location that can be targeted by explicit control
         * flow operations such as goto.
         */
        Label,
        /** @brief A declaration statement.
         *
         * Represents declarations that introduce new entities into the current
         * scope.
         */
        Declaration,
        /** @brief A metadata statement.
         *
         * Represents compiler-directed metadata, annotations, or language
         * directives attached to statements.
         */
        Meta,
        /** @brief An invalid or unresolved statement.
         *
         * Represents an error-recovery node created during parsing when a valid
         * statement could not be constructed.
         */
        Invalid,
    };

    template<typename OffsetType = std::size_t>
    class StatementNode : public Node {
       public:
        /** @brief Virtual destructor for derived statement nodes. */
        virtual ~StatementNode() = default;

        /** @brief Returns the statement kind. */
        virtual StatementKind statement_kind() const {
            return StatementKind::Invalid;
        };
    };

    template<typename OffsetType = std::size_t>
    using Statement = std::shared_ptr<StatementNode>;
}  // namespace Z::Zaban::AST

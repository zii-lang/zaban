#pragma once

#include <Z/Zaban/AST/Node.hpp>

namespace Z::Zaban::AST {
    /** @brief Identifies the kind of expression node in the Abstract Syntax
     * Tree.
     *
     * An ExpressionKind represents the fundamental category of an expression
     * node. Expressions are the primary building blocks of computations and can
     * represent values, operations, control flow constructs, memory operations,
     * and language-specific features.
     *
     * The expression kind determines the structure and semantic behavior of an
     * expression during parsing, semantic analysis, and lowering.
     */
    enum class ExpressionKind {
        /** @brief A primary expression.
         *
         * Represents basic expressions such as identifiers and literals.
         */
        Primary,
        /** @brief A grouped expression.
         *
         * Represents an expression enclosed by grouping syntax.
         *
         * Example:
         * @code
         * (a + b)
         * @endcode
         */
        Group,
        /** @brief An indexed access expression.
         *
         * Accesses an element of an indexed value.
         *
         * Example:
         * @code
         * array[index]
         * @endcode
         */
        IndexAccess,
        /** @brief A member access expression.
         *
         * Accesses a member of a structured value.
         *
         * Example:
         * @code
         * object.member
         * @endcode
         */
        MemberAccess,
        /** @brief A function call expression.
         *
         * Invokes a callable expression with arguments.
         *
         * Example:
         * @code
         * function(args)
         * @endcode
         */
        CallAccess,
        /** @brief A suffix unary expression.
         *
         * Represents postfix operations applied after an expression.
         *
         * Example:
         * @code
         * value++
         * @endcode
         */
        Suffix,
        /** @brief A prefix unary expression.
         *
         * Represents unary operations applied before an expression.
         *
         * Example:
         * @code
         * -value
         * @endcode
         */
        Prefix,
        /** @brief A binary operation expression.
         *
         * Represents an operation combining two operands.
         *
         * Example:
         * @code
         * a + b
         * @endcode
         */
        Binary,
        /** @brief An assignment expression.
         *
         * Represents assigning a value to a writable expression.
         *
         * Example:
         * @code
         * value += 10
         * @endcode
         */
        Assignment,
        /** @brief A compile-time metadata expression.
         *
         * Represents language metadata, annotations, or compiler-directed
         * expressions.
         */
        Meta,
        /** @brief A function expression.
         *
         * Represents an anonymous or inline function value.
         *
         * Example:
         * @code
         * func(a: i32) { ... }
         * @endcode
         */
        Function,
        /** @brief A conditional expression.
         *
         * Represents conditional evaluation constructs.
         *
         * Example:
         * @code
         * if condition { ... }
         * @endcode
         */
        Conditional,
        /** @brief A loop expression.
         *
         * Represents iterative execution constructs.
         */
        Loop,
    };

    /** @brief Base interface for all expression AST nodes.
     *
     * IExpr represents the common interface shared by all expressions in the
     * AST. Expressions are constructs that produce values, such as literals,
     * identifiers, function calls, operators, and assignments.
     *
     * Type information is intentionally not stored directly in the AST. The
     * resolved type of an expression should be managed separately by semantic
     * analysis through external type tables or binding information.
     */
    template<typename OffsetType = std::size_t>
    class ExpressionNode : public Node {
       public:
        /** @brief Virtual destructor for derived expression nodes. */
        virtual ~ExpressionNode() = default;

        /** @brief Returns the expression kind. */
        virtual ExpressionKind expr_kind() const = 0;

        const NodeKind node_kind() const override {
            return NodeKind::Expression;
        }
    };

    template<typename OffsetType = std::size_t>
    using Expression = std::shared_ptr<ExpressionNode>;
}  // namespace Z::Zaban::AST

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Z::Zaban::Langs::ZLang {

#pragma region ForwardDeclarations
    class IExpr;
    class IDeclaration;
    class IStatement;
    class ILiteral;
    class IIdentifier;
    class IAnnotation;
    class IConditionLine;

    class ParameterBase;
    class VariantFieldBase;

    class CombinatorHalfConditionBase;
#pragma endregion ForwardDeclarations

#pragma region EnumTypes
    /** @brief Strongly typed identifier for a symbol binding.
     *
     * BindingId uniquely identifies a binding introduced during semantic
     * analysis, such as a variable, function parameter, or other named entity.
     *
     * Using a dedicated type prevents accidental mixing of binding identifiers
     * with other integer-based identifiers.
     */
    enum class BindingId : std::uint32_t {};

    /** @brief Strongly typed identifier for a lexical scope.
     *
     * ScopeId uniquely identifies a lexical scope within the program. Scope
     * identifiers are used to track scope ownership, nesting relationships, and
     * name resolution context.
     *
     * Using a dedicated type prevents accidental mixing of scope identifiers
     * with other integer-based identifiers.
     */
    enum class ScopeId : std::uint32_t {};

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

    /** @brief Identifies the kind of a literal.
     *
     * A LiteralKind specifies the type of constant value represented by a
     * literal expression in the Abstract Syntax Tree (AST). Literal expressions
     * may represent primitive constants or aggregate values constructed
     * directly in source code.
     */
    enum class LiteralKind {
        /// A null literal.
        Null,
        /// A boolean literal (`true` or `false`).
        Boolean,
        /// A numeric literal (e.g. integer or floating-point).
        Numeric,
        /// A string literal.
        String,
        /// An array literal.
        Array,
        /// A structure literal.
        Struct,
        /// A variant literal.
        Variant,
    };

    /** @brief Identifies the kind of a primary value expression.
     *
     * A PrimaryValueKind represents the fundamental form of a value expression
     * before additional operations or transformations are applied. Primary
     * values are the basic operands from which more complex expressions are
     * constructed.
     */
    enum class PrimaryValueKind {
        /// An identifier reference, a symbol name.
        ID,
        /// A literal value (e.g. number, string, boolean, or aggregate
        /// literal).
        Literal,
    };

    /** @brief Identifies the type of assignment operation.
     *
     * An AssignmentOperator represents the operation performed when assigning
     * a value to an existing storage location. It distinguishes between a
     * simple assignment and compound assignments, where the right-hand side
     * value is combined with the current value before being stored.
     *
     * Compound assignment operators are equivalent to applying a binary
     * operator followed by a normal assignment:
     *
     * @code
     * a += b  ->  a = a + b
     * a *= b  ->  a = a * b
     * @endcode
     */
    enum class AssignmentOperator {
        /// Simple assignment operator (`=`).
        None,
        /// Addition assignment operator (`+=`).
        Add,
        /// Subtraction assignment operator (`-=`).
        Sub,
        /// Multiplication assignment operator (`*=`).
        Mul,
        /// Division assignment operator (`/=`).
        Div,
        /// Remainder/modulo assignment operator (`%=`).
        Mod,
        /// Bitwise OR assignment operator (`|=`).
        Or,
        /// Bitwise AND assignment operator (`&=`).
        And,
        /// Bitwise XOR assignment operator (`^=`).
        Xor,
        /// Left shift assignment operator (`<<=`).
        Shl,
        /// Right shift assignment operator (`>>=`).
        Shr,
    };

    /** @brief Identifies a postfix (suffix) unary operator.
     *
     * Suffix operators are applied after an expression and operate on the
     * expression's current value. They typically modify the value after it has
     * been evaluated.
     */
    enum class SuffixOp {
        /// Post-increment operator (`++`).
        AddAdd,
        /// Post-decrement operator (`--`).
        SubSub,
    };

    /** @brief Identifies a prefix unary operator.
     *
     * Prefix operators are applied before an expression is evaluated. They may
     * transform the value, perform a logical operation, or access a different
     * representation of the operand.
     */
    enum class PrefixOp {
        /// Pre-increment operator (`++`).
        AddAdd,
        /// Pre-decrement operator (`--`).
        SubSub,
        /// Arithmetic negation operator (`-`).
        Neg,
        /// Bitwise NOT operator (`~`).
        BNeg,
        /// Logical NOT operator (`!`).
        LNeg,
        /// Dereference operator (`*>`).
        ///
        /// Resolves a pointer value to access the referenced object.
        Deref,
        /// Address-of operator (`&>`).
        ///
        /// Produces the address of an expression.
        AddrOf,
    };

    /** @brief Identifies a binary operation.
     *
     * Binary operators combine two operands to produce a resulting value. They
     * include arithmetic, bitwise, shift, comparison, and logical operations.
     */
    enum class BinaryOp {
        /// Multiplication operator (`*`).
        Mul,
        /// Division operator (`/`).
        Div,
        /// Remainder/modulo operator (`%`).
        Mod,
        /// Addition operator (`+`).
        Add,
        /// Subtraction operator (`-`).
        Sub,
        /// Left shift operator (`<<`).
        Shl,
        /// Right shift operator (`>>`).
        Shr,
        /// Less-than comparison operator (`<`).
        Lt,
        /// Greater-than comparison operator (`>`).
        Gt,
        /// Less-than-or-equal comparison operator (`<=`).
        Lte,
        /// Greater-than-or-equal comparison operator (`>=`).
        Gte,
        /// Equality comparison operator (`==`).
        Eq,
        /// Inequality comparison operator (`!=`).
        Neq,
        /// Bitwise AND operator (`&`).
        And,
        /// Bitwise XOR operator (`^`).
        Xor,
        /// Bitwise OR operator (`|`).
        Or,
        /// Logical AND operator (`&&`).
        Land,
        /// Logical OR operator (`||`).
        Lor,
    };

    /** @brief Identifies a boolean comparison operation.
     *
     * Boolean operators represent relational operations that compare two values
     * and produce a boolean result.
     *
     * Unlike BinaryOp, BooleanOp contains only operations whose result is a
     * logical truth value.
     */
    enum class BooleanOp {
        /// Equality comparison operator (`==`).
        Eq,
        /// Inequality comparison operator (`!=`).
        Neq,
        /// Less-than comparison operator (`<`).
        Lt,
        /// Greater-than comparison operator (`>`).
        Gt,
        /// Less-than-or-equal comparison operator (`<=`).
        Lte,
        /// Greater-than-or-equal comparison operator (`>=`).
        Gte,
    };

    /** @brief Identifies how multiple conditions are combined within a
     * condition line.
     *
     * A ConditionComOp defines the logical relationship between individual
     * condition expressions in a single conditional branch.
     *
     * Unlike normal boolean operators, these operators are part of Z's extended
     * conditional syntax and describe how multiple condition clauses are
     * evaluated.
     */
    enum class ConditionComOp {
        /** @brief Logical AND condition composition (`?&`).
         *
         * All connected conditions must evaluate to true for the condition line
         * to match.
         *
         * Example:
         * @code
         * ?? == 20 ?& == 30
         * @endcode
         *
         * Equivalent to:
         * @code
         * A == 20 && B == 30
         * @endcode
         */
        And,
        /** @brief Logical OR condition composition (`?|`).
         *
         * At least one connected condition must evaluate to true for the
         * condition line to match.
         *
         * Example:
         * @code
         * ?? == 40 ?| == 20
         * @endcode
         *
         * Equivalent to:
         * @code
         * A == 40 || B == 20
         * @endcode
         */
        Or,
    };

    /** @brief Identifies the execution behavior of a conditional line.
     *
     * A ConditionLineOp controls how matching condition lines are related to
     * previous condition lines in the same conditional block.
     *
     * It extends the traditional if/else-if model by allowing independent
     * evaluation and parallel execution semantics.
     */
    enum class ConditionLineOp {
        /** @brief Chain conditional chain (`??`).
         *
         * Represents a standard conditional branch. Only the first matching
         * condition in the chain is executed.
         *
         * Example:
         * @code
         * ?? == 20 ?& == 30 => { ... }
         * ?? == 40 ?| == 20 => { ... }
         * @endcode
         *
         * Equivalent to:
         * @code
         * if (A == 20 && B == 30) { ... }
         * else if (A == 40 || B == 20) { ... }
         * @endcode
         */
        Chain,
        /** @brief Serial independent conditional evaluation (`?!`).
         *
         * Each condition line is evaluated independently in sequence.
         * A successful condition does not prevent following conditions from
         * being evaluated.
         *
         * Example:
         * @code
         * ??  == 20 ?& == 30 => { ... }
         * ?!  == 40 ?| == 20 => { ... }
         * @endcode
         *
         * Equivalent to:
         * @code
         * if (A == 20 && B == 30) { ... }
         * if (A == 40 || B == 20) { ... }
         * @endcode
         */
        Serial,
        /** @brief Parallel conditional evaluation (`!!`).
         *
         * Each condition line is evaluated as an independent execution unit,
         * allowing matching branches to execute concurrently.
         *
         * Example:
         * @code
         * ?? == 20 ?& == 30 => { ... }
         * !! == 40 ?| == 20 => { ... }
         * @endcode
         *
         * Equivalent to:
         * @code
         * Thread(if (A == 20 && B == 30) { ... });
         * Thread(if (A == 40 || B == 20) { ... });
         * @endcode
         */
        Parallel,
    };

    /** @brief Defines how condition subject values are supplied to
     * half-condition expression.
     *
     * A ConditionArgMode controls the relationship between values declared in
     * the condition header and the arguments used by the expression evaluated
     * inside a condition line.
     *
     * Condition arguments allow conditions to invoke functions
     * using values captured by the surrounding condition statement.
     *
     * Example:
     * @code
     * if a, b
     *     ?? [] add == 30 => { ... }
     * endif;
     * @endcode
     *
     * The values `a` and `b` are passed according to the selected mode.
     */
    enum class ConditionArgMode {
        /** @brief No explicit argument passing mode.
         *
         * Represents a regular condition where the condition expression does
         * not receive values from the condition header.
         */
        None,
        /** @brief Pass all condition header values in declaration order (`[]`).
         *
         * All values from the condition header are forwarded to the expression
         * in their original order.
         *
         * Example:
         * @code
         * if a, b
         *     ?? [] add == 30 => { ... }
         * endif;
         * @endcode
         *
         * Equivalent to:
         * @code
         * add(a, b) == 30
         * @endcode
         */
        PassAll,
        /** @brief Pass selected condition header values by position
         * (`(index...)`).
         *
         * Explicit indexes determine which values from the condition header are
         * forwarded to the expression and in which order.
         *
         * Indexes refer to the position of values in the condition header.
         *
         * Example:
         * @code
         * if a, b
         *     ?? (1, 0) sub == 20 => { ... }
         * endif;
         * @endcode
         *
         * Equivalent to:
         * @code
         * sub(b, a) == 20
         * @endcode
         */
        Positional,
        /** @brief Pass explicitly specified expressions (`[...]`).
         *
         * Instead of forwarding values from the condition header, explicit
         * expressions are evaluated and passed as arguments.
         *
         * Example:
         * @code
         * if a, b
         *     ?? [a, b, 5] sum3 == 35 => { ... }
         * endif;
         * @endcode
         *
         * Equivalent to:
         * @code
         * sum3(a, b, 5) == 35
         * @endcode
         */
        Exact,
    };

    /** @brief Identifies the kind of postfix expression operation.
     *
     * A PostfixKind describes operations that are applied after a primary
     * expression has been parsed. Postfix expressions extend a base expression
     * by accessing elements, members, or invoking callable values.
     *
     * Examples:
     * @code
     * value          -> None
     * array[index]   -> IndexAccess
     * object.member  -> MemberAccess
     * func(args)     -> CallAccess
     * @endcode
     */
    enum class PostfixKind {
        /** @brief No postfix operation.
         *
         * Represents a primary expression or grouped expression that does not
         * perform any additional access operation.
         */
        None,
        /** @brief Index access operation.
         *
         * Accesses an element of an indexed value.
         *
         * Example:
         * @code
         * array[index]
         * @endcode
         */
        IndexAccess,
        /** @brief Member access operation.
         *
         * Accesses a member of a structured value.
         *
         * Example:
         * @code
         * object.member
         * @endcode
         */
        MemberAccess,
        /** @brief Call access operation.
         *
         * Invokes a callable expression with arguments.
         *
         * Example:
         * @code
         * function(args)
         * @endcode
         */
        CallAccess,
    };

    /** @brief Identifies a control-flow transfer operation.
     *
     * A FlowKind represents statements that alter the normal execution flow of
     * a function or loop. These operations are handled during semantic analysis
     * and later lowered into control-flow graph operations.
     */
    enum class FlowKind {
        /** @brief Continue the current loop iteration.
         *
         * Transfers execution to the next iteration point of the nearest
         * enclosing loop.
         */
        Continue,
        /** @brief Exit the current loop.
         *
         * Transfers execution outside the nearest enclosing loop.
         */
        Break,
        /** @brief Jump to a labeled statement.
         *
         * Performs an explicit control-flow transfer to a target label.
         */
        Goto,
        /** @brief Return from the current function.
         *
         * Optionally transfers a return value to the caller depending on the
         * function return type.
         */
        Return,
    };

    /** @brief Identifies the kind of expression node in the Abstract Syntax
     * Tree.
     *
     * An ExprKind represents the fundamental category of an expression node.
     * Expressions are the primary building blocks of computations and can
     * represent values, operations, control flow constructs, memory operations,
     * and language-specific features.
     *
     * The expression kind determines the structure and semantic behavior of an
     * expression during parsing, semantic analysis, and lowering.
     */
    enum class ExprKind {
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
        /** @brief A memory allocation expression.
         *
         * Represents creation of dynamically allocated storage.
         */
        Alloc,  // TODO: Remove this we should not have allocation for dynamic
                // languages
    };

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
#pragma endregion EnumTypes

#pragma region Types
    /** @brief Represents the lexical scope path of an identifier.
     *
     * A ScopeSet stores the sequence of lexical scopes that an identifier is
     * nested within. The stored scope marks describe the identifier's position
     * in the scope hierarchy and can be used during name lookup and resolution.
     */
    struct ScopeSet {
        std::vector<ScopeId> marks;

        /** @brief Adds a scope mark to the scope path. */
        void add(ScopeId id) {
            marks.push_back(id);
        }

        /** @brief Returns whether the scope path contains no marks. */
        bool empty() const {
            return marks.empty();
        }
    };

    /** @brief Represents a named parameter declaration.
     *
     * ParameterBase stores the information required to describe a parameter,
     * including its name, optional type annotation, optional initializer, and
     * variadic state.
     *
     * Parameters can be used for function arguments, structure fields, and
     * other language constructs that introduce named values.
     */
    class ParameterBase {
       private:
        std::string _name;

        // Optional type annotation.
        Annotation _annotation = nullptr;

        // Optional default initializer.
        Expr _initializer = nullptr;

        // Whether this parameter accepts variadic arguments.
        bool _is_vararg = false;

        // Binding assigned during semantic analysis.
        std::optional<BindingId> _binding = std::nullopt;

       public:
        /** @brief Creates a parameter with only a name. */
        ParameterBase(std::string name) : _name(name) {
        }

        /** @brief Creates a parameter with a type annotation. */
        ParameterBase(std::string name, Annotation annotation) :
            _name(name), _annotation(annotation) {
        }

        /** @brief Creates a parameter with a default initializer expression. */
        ParameterBase(std::string name, Expr initializer) :
            _name(name), _initializer(initializer) {
        }

        /** @brief Creates a parameter with a type annotation and initializer.
         */
        ParameterBase(std::string name, Annotation annotation,
                      Expr initializer) :
            _name(name), _annotation(annotation), _initializer(initializer) {
        }

        /** @brief Creates a variadic parameter. */
        ParameterBase(std::string name, bool is_vararg = true) :
            _name(name), _is_vararg(is_vararg) {
        }

        /** @brief Returns the parameter name. */
        std::string get_name() const {
            return _name;
        }

        /** @brief Returns whether this parameter is variadic. */
        bool is_vararg() const {
            return _is_vararg;
        }

        /** @brief Returns the parameter type annotation, if available. */
        Annotation get_annotation() const {
            return _annotation;
        }

        /** @brief Returns the initializer expression, if available. */
        Expr get_initializer() const {
            return _initializer;
        }

        /** @brief Returns the semantic binding associated with this parameter.
         */
        std::optional<BindingId> binding() const {
            return _binding;
        }

        /** @brief Assigns the semantic binding identifier to this parameter. */
        void set_binding(BindingId id) {
            _binding = id;
        }
    };

    /** @brief Represents a field alternative in a variant type.
     *
     * VariantFieldBase describes a single variant alternative, including its
     * name and optional associated parameters.
     *
     * Variant fields with parameters can represent data-carrying variants.
     */
    class VariantFieldBase {
       private:
        std::string            _name;
        std::vector<Parameter> _parameters;

       public:
        /** @brief Creates a variant field with a name and optional parameters.
         */
        VariantFieldBase(std::string name, std::vector<Parameter> parameters) :
            _name(std::move(name)), _parameters(std::move(parameters)) {
        }

        /** @brief Returns the variant field name. */
        const std::string& get_name() const {
            return _name;
        }

        /** @brief Returns the parameters associated with this variant field. */
        const std::vector<Parameter>& get_parameters() const {
            return _parameters;
        }

        /** @brief Returns whether this variant field contains parameters. */
        bool has_parameters() const {
            return !_parameters.empty();
        }
    };
#pragma endregion Types

#pragma region SharedRef
    /** @brief Shared reference to a type annotation node.
     *
     * Annotation represents a type expression in the Abstract Syntax Tree.
     * Type annotations describe the structure and composition of types,
     * including base types, pointers, arrays, and chained function signatures.
     * The underlying IAnnotation node is managed through shared ownership,
     * allowing annotation nodes to be referenced by multiple AST structures
     * while their lifetime is automatically managed.
     */
    using Annotation = std::shared_ptr<IAnnotation>;
    /** @brief Shared reference to a parameter declaration.
     *
     * Parameter provides shared ownership semantics for ParameterBase nodes in
     * the AST. Parameters represent named values with optional type
     * annotations, initializers, or variadic behavior.
     */
    using Parameter = std::shared_ptr<ParameterBase>;
    /** @brief Shared reference to a variant field declaration.
     *
     * VariantField provides shared ownership semantics for VariantFieldBase
     * nodes in the AST. Variant fields represent the possible alternatives
     * declared within a variant type.
     */
    using VariantField = std::shared_ptr<VariantFieldBase>;
    /** @brief Shared reference to a declaration AST node.
     *
     * Declaration provides shared ownership semantics for IDeclaration nodes in
     * the AST. Declarations introduce named entities into a scope, such as type
     * declarations and value bindings.
     */
    using Declaration = std::shared_ptr<IDeclaration>;
    /** @brief Shared reference to a statement AST node.
     *
     * Statement provides shared ownership semantics for IStatement nodes in the
     * AST. Statements represent executable constructs such as expressions,
     * control flow operations, blocks, declarations, and compiler metadata.
     */
    using Statement = std::shared_ptr<IStatement>;
    /** @brief Shared reference to an expression AST node.
     *
     * Expr provides shared ownership semantics for IExpr nodes. Multiple AST
     * structures may reference the same expression node while the node lifetime
     * is managed automatically through reference counting.
     */
    using Expr = std::shared_ptr<IExpr>;
    /** @brief Shared reference to an identifier AST node.
     *
     * Identifier provides shared ownership semantics for IIdentifier nodes.
     * Identifiers represent references to named entities within the program.
     */
    using Identifier = std::shared_ptr<IIdentifier>;
    /** @brief Shared reference to a literal AST node.
     *
     * Literal provides shared ownership semantics for ILiteral nodes.
     * Literals represent constant values such as numbers, strings, booleans,
     * arrays, and structured values.
     */
    using Literal = std::shared_ptr<ILiteral>;
    /** @brief Represents the underlying value stored by a literal node.
     *
     * LiteralValue stores the possible compile-time values that can be
     * represented by a literal expression.
     *
     * The first alternative represents a null literal. Other alternatives
     * represent primitive values and aggregate literal values.
     */
    using LiteralValue =
        std::variant<std::monostate,  // null literal
                     bool,            // boolean literal
                     std::string,     // numeric, string, or named literal value
                     std::vector<Literal>,   // array literal
                     std::vector<Parameter>  // struct literal
                     >;
    /** @brief Represents the value of a primary expression.
     *
     * PrimaryExprValue stores the possible forms of a primary expression:
     * either an identifier reference or a literal value.
     */
    using PrimaryExprValue = std::variant<Identifier, Literal>;

    /** @brief Shared reference to a condition line node.
     *
     * ConditionLine provides shared ownership semantics for IConditionLine
     * nodes. Condition lines represent individual branches of conditional
     * expressions and contain the logic required to evaluate conditional cases.
     */
    using ConditionLine = std::shared_ptr<IConditionLine>;
    /** @brief Shared reference to a combinator half-condition node.
     *
     * CombinatorHalfCondition provides shared ownership semantics for
     * CombinatorHalfConditionBase nodes.
     *
     * Half-conditions represent individual conditional checks that can be
     * combined using condition combinators such as logical AND and OR.
     */
    using CombinatorHalfCondition =
        std::shared_ptr<CombinatorHalfConditionBase>;

#pragma endregion SharedRef

#pragma region Annotations
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
    class IAnnotation : public std::enable_shared_from_this<IAnnotation> {
       public:
        /** @brief Virtual destructor.
         *
         * Ensures proper destruction of derived annotation nodes through a base
         * class pointer.
         */
        virtual ~IAnnotation() = default;

        /** @brief Returns the kind of this annotation node.
         *
         * @return The AnnotationKind identifying the concrete annotation type.
         */
        virtual AnnotationKind get_kind() const = 0;

        /** @brief Casts this annotation to a concrete annotation type.
         *
         * Provides a convenient way to access derived annotation classes while
         * preserving shared ownership semantics.
         *
         * @tparam T The target annotation type.
         *
         * @return A shared pointer to the requested annotation type.
         */
        template<typename T>
        std::shared_ptr<T> cast() {
            return std::static_pointer_cast<T>(shared_from_this());
        }

        // virtual void accept(Visitors::ASTVisitor *visitor) = 0;
    };

    /** @brief Base interface for annotations representing concrete base types.
     *
     * Base annotations describe fundamental types such as primitives,
     * identifiers, enums, structs, and variants.
     */
    class IBaseAnnotation : public IAnnotation {
       public:
        /** @brief Returns the category of the base type.
         */
        virtual BaseAnnotationKind get_base_kind() const = 0;
    };

    /** @brief Represents a primitive type annotation.
     *
     * PrimitiveAnnotation describes built-in language types such as integers,
     * floating-point types, booleans, and other fundamental types.
     */
    class PrimitiveAnnotation : public IBaseAnnotation {
       private:
        std::string name;

       public:
        /** @brief Creates a primitive annotation with the given type name. */
        explicit PrimitiveAnnotation(std::string n) : name(std::move(n)) {
        }

        /** @brief Returns the annotation category. */
        AnnotationKind get_kind() const override {
            return AnnotationKind::Base;
        }

        /** @brief Returns the base type category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Primitive;
        }

        /** @brief Returns the primitive type name. */
        std::string get_name() const {
            return name;
        }

        // void accept(Visitors::ASTVisitor* v) override {
        //     v->visit(this);
        // }
    };

    /** @brief Represents a named type annotation.
     *
     * IdentifierAnnotation describes a type referenced by name. The referenced
     * type is resolved during semantic analysis.
     */
    class IdentifierAnnotation : public IBaseAnnotation {
       private:
        std::string id;

        // Lexical scope path where this identifier is referenced.
        ScopeSet _scope_set;

        // Binding resolved during semantic analysis.
        std::optional<BindingId> _binding = std::nullopt;

       public:
        /** @brief Creates an identifier annotation with the given name. */
        explicit IdentifierAnnotation(std::string i) : id(std::move(i)) {
        }

        /** @brief Returns the annotation category. */
        AnnotationKind get_kind() const override {
            return AnnotationKind::Base;
        }

        /** @brief Returns the base annotation category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Identifier;
        }

        /** @brief Returns the referenced identifier name. */
        std::string get_id() const {
            return id;
        }

        /** @brief Returns the lexical scope path of this identifier. */
        ScopeSet& scope_set() {
            return _scope_set;
        }

        /** @brief Returns the lexical scope path of this identifier. */
        const ScopeSet& scope_set() const {
            return _scope_set;
        }

        /** @brief Assigns the resolved semantic binding. */
        void set_binding(BindingId id) {
            _binding = id;
        }

        /** @brief Returns the resolved binding, if available. */
        std::optional<BindingId> binding() const {
            return _binding;
        }
    };

    /** @brief Represents an enum type annotation with its declared fields. */
    class EnumAnnotation : public IBaseAnnotation {
       private:
        std::vector<Parameter> fields;

       public:
        explicit EnumAnnotation(std::vector<Parameter> f) :
            fields(std::move(f)) {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Base;
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Enum;
        }

        /** @brief Returns the enum fields. */
        const std::vector<Parameter>& get_fields() const {
            return fields;
        }
    };

    /** @brief Represents a struct type annotation with its declared fields. */
    class StructAnnotation : public IBaseAnnotation {
       private:
        std::vector<Parameter> fields;

       public:
        explicit StructAnnotation(std::vector<Parameter> f) :
            fields(std::move(f)) {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Base;
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Struct;
        }

        /** @brief Returns the struct fields. */
        const std::vector<Parameter>& get_fields() const {
            return fields;
        }
    };

    /** @brief Represents a variant type annotation with its possible variants.
     */
    class VariantAnnotation : public IBaseAnnotation {
       private:
        std::vector<VariantField> variants;

       public:
        explicit VariantAnnotation(std::vector<VariantField> v) :
            variants(std::move(v)) {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Base;
        }

        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Variant;
        }

        /** @brief Returns the variant alternatives. */
        const std::vector<VariantField>& get_variants() const {
            return variants;
        }
    };

    /** @brief Represents a pointer type annotation.
     *
     * Wraps another annotation as the pointed-to type.
     */
    class PointerAnnotation : public IAnnotation {
       private:
        Annotation pointee;

       public:
        explicit PointerAnnotation(Annotation p) : pointee(std::move(p)) {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Pointer;
        }

        /** @brief Returns the pointed-to annotation. */
        Annotation get_pointee() const {
            return pointee;
        }
    };

    /** @brief Represents an array type annotation.
     *
     * Stores the element type and optional array size expression.
     */
    class ArrayAnnotation : public IAnnotation {
        Annotation element;
        Expr       num_elements;

       public:
        explicit ArrayAnnotation(Annotation e, Expr num_elements) :
            element(std::move(e)), num_elements(num_elements) {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Array;
        }

        /** @brief Returns the array element annotation. */
        Annotation get_element() const {
            return element;
        }

        /** @brief Returns the expression defining the number of elements. */
        Expr get_num_elements() const {
            return num_elements;
        }
    };

    /** @brief Represents a chained type annotation.
     *
     * Used to represent composed types such as function signatures:
     *
     * i32 -> i32 -> void
     */
    class ChainAnnotation : public IAnnotation {
        Annotation _from;
        Annotation _to;

       public:
        ChainAnnotation(Annotation from, Annotation to) :
            _from(std::move(from)), _to(std::move(to)) {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Chain;
        }

        /** @brief Returns the input side of the chain. */
        const Annotation& get_from() const {
            return _from;
        }

        /** @brief Returns the output side of the chain. */
        const Annotation& get_to() const {
            return _to;
        }
    };

    /** @brief Represents a variadic argument type annotation (`...`). */
    class VarargAnnotation : public IAnnotation {
       public:
        VarargAnnotation() {
        }

        AnnotationKind get_kind() const override {
            return AnnotationKind::Vararg;
        }
    };

#pragma endregion Annotations

#pragma region Declartions
    /** @brief Base interface for declaration AST nodes.
     *
     * IDeclaration represents entities that introduce names into a scope, such
     * as type declarations and variable declarations.
     */
    class IDeclaration : public std::enable_shared_from_this<IDeclaration> {
       public:
        /** @brief Virtual destructor for derived declaration nodes. */
        virtual ~IDeclaration() = default;

        /** @brief Returns the declaration kind. */
        virtual const DeclarationKind kind() const = 0;

        template<typename T>
        inline std::shared_ptr<T> cast() {
            return std::static_pointer_cast<T>(shared_from_this());
        }
    };

    /** @brief Represents a type declaration.
     *
     * TypeDeclaration introduces a named type into the current scope. The
     * declaration contains the type name, its associated annotation, and the
     * semantic binding assigned during name resolution.
     *
     * Example:
     * @code
     * type Name : i32;
     * @endcode
     */
    class TypeDeclaration : public IDeclaration {
       private:
        const std::string _name;
        const Annotation  _annotation;

        // Binding assigned during semantic analysis.
        std::optional<BindingId> _binding = std::nullopt;

       public:
        TypeDeclaration(std::string name, Annotation annotation) :
            _name(name), _annotation(std::move(annotation)) {
        }

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::TypeDecl;
        }

        /** @brief Returns the declared type name. */
        const std::string get_name() const {
            return _name;
        }

        /** @brief Returns the type annotation. */
        const Annotation get_annotation() const {
            return _annotation;
        }

        /** @brief Returns the resolved binding, if available. */
        std::optional<BindingId> binding() const {
            return _binding;
        }

        /** @brief Assigns the semantic binding for this declaration. */
        void set_binding(BindingId id) {
            _binding = id;
        }
    };

    /** @brief Represents a value binding declaration.
     *
     * LetDeclaration introduces a named value into the current scope. A value
     * declaration may optionally provide a type annotation, an initializer
     * expression, or both.
     *
     * The binding information is assigned during semantic analysis after name
     * resolution.
     *
     * Example:
     * @code
     * let value : i32 = 10;
     * @endcode
     */
    class LetDeclaration : public IDeclaration {
       private:
        const std::string _name;

        // Optional type annotation.
        const Annotation _type = nullptr;

        // Optional initializer expression.
        const Expr _initializer = nullptr;

        // Binding assigned during semantic analysis.
        std::optional<BindingId> _binding = std::nullopt;

       public:
        /** @brief Creates a value declaration with only a name. */
        LetDeclaration(std::string name) : _name(name) {
        }

        /** @brief Creates a value declaration with an explicit type. */
        LetDeclaration(std::string name, Annotation type) :
            _name(name), _type(std::move(type)), _initializer(nullptr) {
        }

        /** @brief Creates a value declaration with an initializer expression.
         */
        LetDeclaration(std::string name, Expr init) :
            _name(name), _initializer(std::move(init)) {
        }

        /** @brief Creates a value declaration with a type and initializer. */
        LetDeclaration(std::string name, Annotation type, Expr init) :
            _name(name), _type(std::move(type)), _initializer(std::move(init)) {
        }

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::LetDecl;
        }

        /** @brief Returns the declared value name. */
        const std::string get_name() const {
            return _name;
        }

        /** @brief Returns the declared value type annotation, if available. */
        const Annotation get_annotation() const {
            return _type;
        }

        /** @brief Returns the initializer expression, if available. */
        const Expr get_initializer() const {
            return _initializer;
        }

        /** @brief Returns the resolved binding, if available. */
        std::optional<BindingId> binding() const {
            return _binding;
        }

        /** @brief Assigns the semantic binding for this declaration. */
        void set_binding(BindingId id) {
            _binding = id;
        }
    };
#pragma endregion Declartions

#pragma region Statements
    /** @brief Base interface for all statement AST nodes.
     *
     * IStatement represents the common interface for statements in the AST.
     * Statements describe executable constructs, declarations, control flow,
     * and other language-level operations.
     */
    class IStatement : public std::enable_shared_from_this<IStatement> {
       public:
        /** @brief Virtual destructor for derived statement nodes. */
        virtual ~IStatement() = default;

        /** @brief Returns the statement kind. */
        virtual StatementKind kind() const = 0;

        template<typename T>
        inline std::shared_ptr<T> cast() {
            return std::static_pointer_cast<T>(shared_from_this());
        }
    };

    /** @brief Represents an invalid or unresolved statement.
     *
     * InvalidStatement is used during parser error recovery when a valid
     * statement node cannot be created.
     */
    class InvalidStatement : public IStatement {
       public:
        InvalidStatement() {
        }

        StatementKind kind() const override {
            return StatementKind::Invalid;
        }
    };

    /** @brief Represents a statement containing an expression.
     *
     * ExpressionStatement wraps an expression that is evaluated as a statement,
     * typically for its side effects.
     */
    class ExpressionStatement : public IStatement {
        const Expr _expression;

       public:
        /** @brief Creates an expression statement. */
        ExpressionStatement(Expr expression) :
            _expression(std::move(expression)) {
        }

        StatementKind kind() const override {
            return StatementKind::Expression;
        }

        /** @brief Returns the contained expression. */
        const Expr get() const {
            return _expression;
        }
    };

    /** @brief Represents a control-flow statement.
     *
     * FlowStatement represents statements that alter normal execution flow,
     * such as return, break, continue, and goto operations.
     *
     * A flow statement may optionally contain a target label or return
     * expression.
     */
    class FlowStatement : public IStatement {
        const FlowKind                   _type;
        const std::optional<std::string> _jmplabel;
        const Expr                       _return_expr;

       public:
        /** @brief Creates a flow statement without additional data. */
        FlowStatement(FlowKind type) :
            _type(type), _jmplabel(std::nullopt), _return_expr(nullptr) {
        }

        /** @brief Creates a flow statement targeting a label. */
        FlowStatement(FlowKind type, std::string label) :
            _type(type), _jmplabel(label), _return_expr(nullptr) {
        }

        /** @brief Creates a return flow statement with an expression. */
        FlowStatement(FlowKind type, Expr return_expr) :
            _type(type), _jmplabel(std::nullopt),
            _return_expr(std::move(return_expr)) {
        }

        StatementKind kind() const override {
            return StatementKind::Flow;
        }

        /** @brief Returns the flow operation kind. */
        FlowKind get_type() const {
            return this->_type;
        }

        /** @brief Returns whether this statement has a jump label. */
        bool has_label() const {
            switch (this->_type) {
                case FlowKind::Return:
                    return false;
                default:
                    return this->_jmplabel.has_value();
            }
        }

        /** @brief Returns the jump target label. */
        std::string get_label() const {
            return this->_jmplabel.value();
        }

        /** @brief Returns whether this statement contains a return expression.
         */
        bool has_return_expr() const {
            if (this->_type == FlowKind::Return &&
                this->_return_expr != nullptr) {
                return true;
            }
            return false;
        }

        /** @brief Returns the associated expression, if available. */
        Expr get_expr() const {
            return this->_return_expr;
        }
    };

    /** @brief Represents a scoped sequence of statements.
     *
     * BlockStatement contains an ordered collection of statements that are
     * evaluated sequentially within a lexical scope.
     *
     * Blocks are used to group statements together and define scope boundaries
     * for declarations and name resolution.
     */
    class BlockStatement : public IStatement {
       private:
        std::vector<Statement> _statements;

       public:
        /** @brief Creates a block statement from a list of statements. */
        BlockStatement(std::vector<Statement>& statements) :
            _statements(statements) {
        }

        /** @brief Returns the number of statements contained in this block. */
        std::size_t statementc() const {
            return this->_statements.size();
        }

        /** @brief Returns an iterator to the first statement in the block. */
        std::vector<Statement>::iterator stmt_begin() {
            return this->_statements.begin();
        }

        /** @brief Returns an iterator past the last statement in the block. */
        std::vector<Statement>::iterator stmt_end() {
            return this->_statements.end();
        }

        /** @brief Returns the statement at the specified index. */
        Statement get_statement(std::size_t pos) {
            return this->_statements[pos];
        }

        /** @brief Returns the statement kind. */
        StatementKind kind() const override {
            return StatementKind::Block;
        }
    };

    /** @brief Represents a label statement.
     *
     * LabelStatement introduces a named control-flow target that can be
     * referenced by jump operations.
     */
    class LabelStatement : public IStatement {
       private:
        const std::string _name;

       public:
        /** @brief Creates a label statement with the given name. */
        LabelStatement(std::string name) : _name(name) {
        }

        /** @brief Returns the statement kind. */
        StatementKind kind() const override {
            return StatementKind::Label;
        }

        /** @brief Returns the label name. */
        std::string get_name() const {
            return this->_name;
        }
    };

    /** @brief Represents a declaration statement.
     *
     * DeclarationStatement wraps a declaration node so that declarations can
     * appear in statement sequences.
     */
    class DeclarationStatement : public IStatement {
       private:
        const Declaration _declaration;

       public:
        /** @brief Creates a declaration statement. */
        DeclarationStatement(Declaration declaration) :
            _declaration(std::move(declaration)) {
        }

        StatementKind kind() const override {
            return StatementKind::Declaration;
        }

        /** @brief Returns the wrapped declaration. */
        const Declaration get() const {
            return this->_declaration;
        }
    };

    /** @brief Represents a metadata statement.
     *
     * MetaStatement stores compiler or language metadata attached to a
     * statement. Metadata may optionally wrap an inner statement that it
     * modifies.
     */
    class MetaStatement : public IStatement {
       private:
        const std::string _name;
        const Statement   _inner = nullptr;

       public:
        /** @brief Creates metadata without an attached statement. */
        MetaStatement(std::string name) : _name(std::move(name)) {
        }

        /** @brief Creates metadata wrapping another statement. */
        MetaStatement(std::string name, Statement inner) :
            _name(std::move(name)), _inner(std::move(inner)) {
        }

        StatementKind kind() const override {
            return StatementKind::Meta;
        }

        /** @brief Returns the metadata name. */
        const std::string& get_name() const {
            return _name;
        }

        /** @brief Returns whether this metadata has an inner statement. */
        bool has_inner() const {
            return _inner != nullptr;
        }

        /** @brief Returns the wrapped statement, if available. */
        Statement get_inner() const {
            return _inner;
        }
    };
#pragma endregion Statements

#pragma region Atomics
    /** @brief Represents a literal value in the AST.
     *
     * ILiteral stores a compile-time constant value together with its literal
     * category. Literals represent values that can be directly written in
     * source code, such as numbers, strings, booleans, arrays, structures, and
     * variants.
     *
     * The stored value is kept in a LiteralValue variant and can be accessed
     * through typed getters based on the literal kind.
     */
    class ILiteral {
       private:
        // Identifies this node as a literal primary value.
        const PrimaryValueKind pkind = PrimaryValueKind::Literal;

        // The specific literal category.
        const LiteralKind kind;

        // The underlying literal data.
        const LiteralValue value;

       public:
        /** @brief Creates a literal with a specific kind and value. */
        ILiteral(LiteralKind kind, LiteralValue value) :
            kind(kind), value(std::move(value)) {
        }

        /** @brief Returns the primary expression category. */
        PrimaryValueKind get_pkind() {
            return this->pkind;
        }

        /** @brief Returns the literal category. */
        LiteralKind get_kind() {
            return this->kind;
        }

        /** @brief Returns the underlying literal value. */
        LiteralValue get_value() {
            return this->value;
        }

        /** @brief Returns the stored value as the requested type. */
        template<typename T>
        T get() {
            return std::get<T>(this->value);
        }

        /** @brief Returns the boolean value if this is a boolean literal. */
        bool get_bool() {
            if (this->kind == LiteralKind::Boolean) {
                return std::get<bool>(this->value);
            }
            return false;
        }

        /** @brief Returns the string representation of numeric or string
         * literals. */
        std::string get_string() {
            switch (this->kind) {
                case LiteralKind::Numeric:
                case LiteralKind::String:
                    return std::get<std::string>(this->value);
                default:
                    return nullptr;
            }
        }

        /** @brief Returns the fields of a structure literal. */
        std::vector<Parameter> get_struct() {
            return std::get<std::vector<Parameter>>(this->value);
        }

        /** @brief Destroys the literal node. */
        ~ILiteral() = default;
    };

    /** @brief Represents an identifier reference in the AST.
     *
     * IIdentifier stores the name of a referenced symbol. Identifiers are
     * resolved during semantic analysis and later associated with their
     * corresponding bindings.
     */
    class IIdentifier {
       private:
        // Identifies this node as an identifier primary value.
        const PrimaryValueKind pkind = PrimaryValueKind::ID;

        // Referenced identifier name.
        std::string _name;

       public:
        /** @brief Creates an identifier with the given name. */
        IIdentifier(std::string&& name) : _name(std::move(name)) {
        }

        /** @brief Returns the primary expression category. */
        PrimaryValueKind get_pkind() {
            return this->pkind;
        }

        /** @brief Returns the referenced identifier name. */
        std::string get_name() const {
            return this->_name;
        }
    };
#pragma endregion Atomics

#pragma region ConditionLine

    /** @brief Represents a single boolean condition check.
     *
     * HalfCondition stores a comparison operation together with its operands.
     * It can represent both unary-style condition checks, where only the
     * right-hand expression is evaluated, and binary comparisons involving a
     * left and right expression.
     *
     * Example:
     * @code
     * == value
     * a == value
     * @endcode
     */
    class HalfCondition {
        BooleanOp _op;
        Expr      _rhs;
        Expr      _lhs = nullptr;

       public:
        /** @brief Creates a condition with only a right-hand expression. */
        HalfCondition(BooleanOp op, Expr&& rhs) :
            _op(op), _rhs(std::move(rhs)) {
        }

        /** @brief Creates a condition with left and right expressions. */
        HalfCondition(BooleanOp op, Expr&& lhs, Expr&& rhs) :
            _op(op), _rhs(std::move(rhs)), _lhs(std::move(lhs)) {
        }

        /** @brief Returns the comparison operator. */
        BooleanOp get_operation() const {
            return this->_op;
        }

        /** @brief Returns the right-hand expression. */
        Expr get_rhs() const {
            return this->_rhs;
        }

        /** @brief Returns the left-hand expression, if present. */
        Expr get_lhs() const {
            return this->_lhs;
        }

        /** @brief Returns the expression used as the primary condition value.
         */
        Expr get_expr() const {
            return this->_rhs;
        }
    };

    /** @brief Represents a condition combined with a condition combinator.
     *
     * CombinatorHalfConditionBase associates a half-condition with a combinator
     * operator that defines how it participates in a conditional expression.
     *
     * Example:
     * @code
     * ?? == 20 ?& == 30
     * @endcode
     *
     * Each condition part is stored together with its logical combination mode,
     * such as AND or OR.
     */
    class CombinatorHalfConditionBase {
        const ConditionComOp _op;
        const HalfCondition  _cond;

       public:
        /** @brief Creates a combinator condition from an operator and
         * condition. */
        CombinatorHalfConditionBase(ConditionComOp op, HalfCondition&& half) :
            _op(op), _cond(std::move(half)) {
        }

        /** @brief Returns the condition combinator operator. */
        ConditionComOp get_operation() const {
            return this->_op;
        }

        /** @brief Returns the associated half-condition. */
        HalfCondition get_cond() const {
            return this->_cond;
        }
    };

    /** @brief Represents a function-based condition expression.
     *
     * FunctionCondition represents a conditional check performed by calling a
     * function with arguments derived from the condition inputs.
     *
     * The argument passing mode controls how condition values are provided to
     * the function:
     *
     * - PassAll: Passes all available condition arguments in order.
     * - Positional: Passes selected arguments by index.
     * - Exact: Passes explicitly provided expressions.
     *
     * A condition may optionally include a comparison that evaluates the
     * returned function value.
     *
     * Example:
     * @code
     * if a, b
     *     ?? [] check == true => { ... }
     * @endcode
     */
    class FunctionCondition {
        ConditionArgMode    _mode;
        std::vector<size_t> _positional_indices;  // for positional mode
        std::vector<Expr>   _exact_args;          // for exact mode
        Expr                _callee;
        std::optional<HalfCondition> _comparison;

       public:
        /** @brief Creates a function condition with the specified argument
         * mode. */
        FunctionCondition(ConditionArgMode mode, Expr&& callee) :
            _mode(mode), _callee(std::move(callee)), _comparison(std::nullopt) {
        }

        /** @brief Creates a function condition using positional arguments. */
        FunctionCondition(std::vector<size_t> indices, Expr&& callee) :
            _mode(ConditionArgMode::Positional),
            _positional_indices(std::move(indices)), _callee(std::move(callee)),
            _comparison(std::nullopt) {
        }

        /** @brief Creates a function condition using explicit arguments. */
        FunctionCondition(std::vector<Expr> args, Expr&& callee) :
            _mode(ConditionArgMode::Exact), _exact_args(std::move(args)),
            _callee(std::move(callee)) {
        }

        /** @brief Creates a function condition with a comparison. */
        FunctionCondition(ConditionArgMode mode, Expr&& callee,
                          HalfCondition cmp) :
            _mode(mode), _callee(std::move(callee)),
            _comparison(std::move(cmp)) {
        }

        /** @brief Creates a positional function condition with a comparison. */
        FunctionCondition(std::vector<size_t> indices, Expr&& callee,
                          HalfCondition cmp) :
            _mode(ConditionArgMode::Positional),
            _positional_indices(std::move(indices)), _callee(std::move(callee)),
            _comparison(std::move(cmp)) {
        }

        /** @brief Creates an exact argument function condition with a
         * comparison. */
        FunctionCondition(std::vector<Expr> args, Expr callee,
                          HalfCondition cmp) :
            _mode(ConditionArgMode::Exact), _exact_args(std::move(args)),
            _callee(std::move(callee)), _comparison(std::move(cmp)) {
        }

        /** @brief Returns the argument passing mode. */
        ConditionArgMode get_mode() const {
            return _mode;
        }

        /** @brief Returns positional argument indices. */
        const std::vector<size_t>& get_positional_indices() const {
            return _positional_indices;
        }

        /** @brief Returns explicitly specified arguments. */
        const std::vector<Expr>& get_exact_args() const {
            return _exact_args;
        }

        /** @brief Returns the function expression being called. */
        Expr get_callee() const {
            return _callee;
        }

        /** @brief Returns whether this condition has a comparison. */
        bool has_comparison() const {
            return _comparison.has_value();
        }

        /** @brief Returns the comparison applied to the function result. */
        const HalfCondition& get_comparison() const {
            return _comparison.value();
        }
    };

    /** @brief Represents a single conditional branch line.
     *
     * IConditionLine represents one branch of a conditional statement. A
     * condition line may contain a direct condition, a function-based
     * condition, optional combinator conditions, and the statement executed
     * when the condition matches.
     *
     * The operation determines how this line participates in the surrounding
     * conditional structure:
     *
     * - Canon: regular conditional branch.
     * - Serial: evaluates as an independent conditional branch.
     * - Parallel: evaluates concurrently with other branches.
     */
    class IConditionLine {
        ConditionLineOp                      _op;
        std::optional<HalfCondition>         _base;
        std::optional<FunctionCondition>     _func_cond;
        std::vector<CombinatorHalfCondition> _comb;
        Statement                            _stmt;

       public:
        /** @brief Creates a condition line without a condition. */
        IConditionLine(ConditionLineOp op, Statement&& stmt) :
            _op(op), _stmt(std::move(stmt)), _base(std::nullopt), _comb() {
        }

        /** @brief Creates a condition line with a base condition. */
        IConditionLine(ConditionLineOp op, Statement&& stmt,
                       HalfCondition&& base) :
            _op(op), _stmt(std::move(stmt)), _base(std::move(base)), _comb() {
        }

        /** @brief Creates a condition line with a base and combinator
         * conditions. */
        IConditionLine(ConditionLineOp op, Statement&& stmt,
                       HalfCondition&&                        base,
                       std::vector<CombinatorHalfCondition>&& comb) :
            _op(op), _stmt(std::move(stmt)), _base(std::move(base)),
            _comb(std::move(comb)) {
        }

        /** @brief Creates a condition line with a function condition. */
        IConditionLine(ConditionLineOp op, Statement&& stmt,
                       FunctionCondition&& func_cond) :
            _op(op), _stmt(std::move(stmt)), _func_cond(std::move(func_cond)),
            _base(std::nullopt), _comb() {
        }

        /** @brief Returns the condition line operation mode. */
        ConditionLineOp get_operation() const {
            return this->_op;
        }

        /** @brief Returns the statement executed by this condition line. */
        Statement get_statement() const {
            return this->_stmt;
        }

        /** @brief Returns whether this line has a base condition. */
        bool has_base() const {
            return this->_base.has_value();
        }

        /** @brief Returns whether this line has a function condition. */
        bool has_func_cond() const {
            return this->_func_cond.has_value();
        }

        /** @brief Returns the function condition. */
        const FunctionCondition& get_func_cond() const {
            return _func_cond.value();
        }

        /** @brief Returns the base condition. */
        std::optional<HalfCondition> get_base() const {
            return this->_base.value();
        }

        /** @brief Returns the number of combinator conditions. */
        std::size_t get_combc() const {
            return this->_comb.size();
        }

        /** @brief Returns a combinator condition by index. */
        CombinatorHalfCondition get_comb_at(std::size_t pos) {
            return this->_comb[pos];
        }

        /** @brief Returns an iterator to the first combinator condition. */
        std::vector<CombinatorHalfCondition>::iterator comb_begin() {
            return this->_comb.begin();
        }

        /** @brief Returns an iterator past the last combinator condition. */
        std::vector<CombinatorHalfCondition>::iterator comb_end() {
            return this->_comb.end();
        }
    };

#pragma endregion

#pragma region Expressions

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
    class IExpr : public std::enable_shared_from_this<IExpr> {
       public:
        /** @brief Virtual destructor for derived expression nodes. */
        virtual ~IExpr() = default;

        /** @brief Returns the expression kind. */
        virtual ExprKind kind() const = 0;

        /**
         * @brief Casts this expression node to a derived expression type.
         *
         * The caller must ensure that the requested type matches the actual
         * expression node type.
         */
        template<typename T>
        inline std::shared_ptr<T> cast() {
            return std::static_pointer_cast<T>(shared_from_this());
        }
    };

    /** @brief Represents a primary expression.
     *
     * PrimaryExpr represents the simplest form of expression in the AST,
     * including identifier references and literal values.
     *
     * Identifier expressions store lexical scope information and are resolved
     * during semantic analysis by assigning a corresponding binding. Literal
     * expressions do not require name resolution.
     */
    class PrimaryExpr : public IExpr {
        const PrimaryExprValue value;

        // Resolution information for identifier expressions.
        // Unused for literal expressions.
        ScopeSet                 _scope_set;
        std::optional<BindingId> _binding = std::nullopt;

       public:
        /** @brief Creates a primary expression from an identifier. */
        PrimaryExpr(Identifier&& id) : value(id) {
        }

        /** @brief Creates a primary expression from a literal. */
        PrimaryExpr(Literal&& literal) : value(literal) {
        }

        /** @brief Creates a primary expression from an identifier name. */
        PrimaryExpr(std::string&& id) :
            value(std::make_shared<IIdentifier>(id)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Primary;
        }

        /** @brief Returns the stored primary value as the requested type.
         *
         * The requested type must match the active alternative stored in the
         * primary expression value.
         */
        template<typename T>
        T get() const {
            return std::get<T>(this->value);
        }

        /** @brief Returns whether this expression contains an identifier or
         * literal. */
        PrimaryValueKind value_kind() const {
            if (std::holds_alternative<Literal>(value)) {
                return PrimaryValueKind::Literal;
            }
            return PrimaryValueKind::ID;
        }

        /** @brief Returns the lexical scope path of this identifier expression.
         */
        ScopeSet& scope_set() {
            return _scope_set;
        }

        /** @brief Returns the lexical scope path of this identifier expression.
         */
        const ScopeSet& scope_set() const {
            return _scope_set;
        }

        /** @brief Assigns the resolved binding for this identifier expression.
         */
        void set_binding(BindingId id) {
            _binding = id;
        }

        /** @brief Returns the resolved binding, if available. */
        std::optional<BindingId> binding() const {
            return _binding;
        }
    };

    /** @brief Represents a parenthesized expression.
     *
     * Stores an inner expression while preserving grouping information from the
     * source code.
     */
    class GroupExpr : public IExpr {
        const Expr value;

       public:
        /** @brief Creates a grouped expression. */
        GroupExpr(Expr&& expr) : value(expr) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Group;
        }

        /** @brief Returns the inner expression. */
        Expr get() const {
            return this->value;
        }
    };

    /** @brief Represents a binary operation expression.
     *
     * BinaryExpr stores an operator and two operand expressions. The operation
     * is evaluated by applying the operator to the left and right expressions.
     */
    class BinaryExpr : public IExpr {
       private:
        const BinaryOp _op;
        const Expr     _left;
        const Expr     _right;

       public:
        /** @brief Creates a binary expression. */
        BinaryExpr(Expr left, Expr right, BinaryOp op) :
            _op(op), _left(std::move(left)), _right(std::move(right)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Binary;
        }

        /** @brief Returns the binary operator. */
        BinaryOp get_op() const {
            return this->_op;
        }

        /** @brief Returns the left operand expression. */
        const Expr get_left() const {
            return this->_left;
        }

        /** @brief Returns the right operand expression. */
        const Expr get_right() const {
            return this->_right;
        }
    };

    /** @brief Represents an indexed access expression.
     *
     * IndexAccessExpr represents accessing an element from an indexed value,
     * such as an array or other indexable object.
     *
     * Example:
     * @code
     * values[index]
     * @endcode
     */
    class IndexAccessExpr : public IExpr {
       private:
        const Expr _expr;
        const Expr _access;

       public:
        /** @brief Creates an indexed access expression. */
        IndexAccessExpr(Expr expr, Expr access) :
            _expr(std::move(expr)), _access(std::move(access)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::IndexAccess;
        };

        /** @brief Returns the expression being indexed. */
        Expr expr() const {
            return this->_expr;
        }

        /** @brief Returns the index expression. */
        Expr access() const {
            return this->_access;
        }
    };

    /** @brief Represents a member access expression.
     *
     * MemberAccessExpr represents accessing a named member from a base
     * expression.
     *
     * Example:
     * @code
     * base.member
     * @endcode
     */
    class MemberAccessExpr : public IExpr {
        Expr        _base;
        std::string _member;

       public:
        /** @brief Creates a member access expression. */
        MemberAccessExpr(Expr base, std::string member) :
            _base(std::move(base)), _member(member) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::MemberAccess;
        }

        /** @brief Returns the base expression. */
        Expr get_base() const {
            return this->_base;
        }

        /** @brief Returns the accessed member name. */
        std::string get_member() const {
            return this->_member;
        }
    };

    /** @brief Represents a call expression.
     *
     * CallAccessExpr represents invoking a callable expression with a list of
     * argument expressions.
     *
     * Example:
     * @code
     * callee(arg, ...)
     * @endcode
     */
    class CallAccessExpr : public IExpr {
        Expr              _callee;
        std::vector<Expr> _args;

       public:
        /** @brief Creates a function call expression. */
        CallAccessExpr(Expr callee, std::vector<Expr>&& args) :
            _callee(std::move(callee)), _args(args) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::CallAccess;
        }

        /** @brief Returns the callable expression. */
        Expr get_callee() const {
            return this->_callee;
        }

        /** @brief Returns the number of arguments. */
        std::size_t get_argc() const {
            return this->_args.size();
        }

        /** @brief Returns the argument at the specified position. */
        Expr get_arg_at(std::size_t pos) {
            return this->_args[pos];
        }

        /** @brief Returns an iterator to the first argument. */
        std::vector<Expr>::iterator arg_begin() {
            return this->_args.begin();
        }

        /** @brief Returns an iterator past the last argument. */
        std::vector<Expr>::iterator arg_end() {
            return this->_args.end();
        }
    };

    /** @brief Represents a postfix operation expression.
     *
     * SuffixExpr represents an expression followed by a postfix operator, where
     * the operand is evaluated with the operator applied after its value is
     * used.
     */
    class SuffixExpr : public IExpr {
        const SuffixOp _op;
        const Expr     _expr;

       public:
        /** @brief Creates a postfix expression. */
        SuffixExpr(Expr expr, SuffixOp operation) :
            _expr(std::move(expr)), _op(operation) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Suffix;
        }

        /** @brief Returns the postfix operator. */
        SuffixOp get_operation() const {
            return this->_op;
        }

        /** @brief Returns the operand expression. */
        Expr get_expr() const {
            return this->_expr;
        }
    };

    /** @brief Represents a prefix operation expression.
     *
     * PrefixExpr represents an expression with a prefix operator applied before
     * evaluating the operand.
     */
    class PrefixExpr : public IExpr {
        const PrefixOp _op;
        const Expr     _expr;

       public:
        /** @brief Creates a prefix expression. */
        PrefixExpr(Expr expr, PrefixOp op) : _op(op), _expr(expr) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Prefix;
        }

        /** @brief Returns the prefix operator. */
        PrefixOp get_operation() const {
            return this->_op;
        }

        /** @brief Returns the operand expression. */
        Expr get_expr() const {
            return this->_expr;
        }
    };

    /** @brief Represents an assignment expression.
     *
     * AssignmentExpr represents assigning a value or type annotation to a
     * target expression.
     *
     * The right-hand side may contain either an expression value or an
     * annotation, allowing assignments to represent both runtime value
     * assignment and type-related assignment forms.
     */
    class AssignmentExpr : public IExpr {
        const AssignmentOperator             _op;
        const Expr                           _left;
        const std::variant<Expr, Annotation> _right;

       public:
        /** @brief Creates an assignment expression with an expression value. */
        AssignmentExpr(AssignmentOperator operation, Expr left, Expr right) :
            _op(operation), _left(std::move(left)), _right(std::move(right)) {
        }

        /** @brief Creates an assignment expression with a type annotation. */
        AssignmentExpr(AssignmentOperator operation, Expr left,
                       Annotation right) :
            _op(operation), _left(std::move(left)), _right(std::move(right)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Assignment;
        }

        /** @brief Returns the assignment operator. */
        AssignmentOperator get_operation() const {
            return this->_op;
        }

        /** @brief Returns the assignment target expression. */
        Expr get_left() const {
            return this->_left;
        }

        /** @brief Returns the assigned value or annotation. */
        std::variant<Expr, Annotation> get_right() const {
            return this->_right;
        }
    };

    /** @brief Represents a metadata expression.
     *
     * MetaExpr represents a compiler or language-level metadata expression with
     * a name and a list of associated arguments.
     *
     * Metadata expressions can be used to attach additional information or
     * control behavior during later compiler stages.
     */
    class MetaExpr : public IExpr {
        const std::string       _name;
        const std::vector<Expr> _args;

       public:
        /** @brief Creates a metadata expression with arguments. */
        MetaExpr(std::string name, std::vector<Expr>&& args) :
            _name(name), _args(args) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Meta;
        }

        /** @brief Returns the metadata name. */
        std::string get_name() const {
            return this->_name;
        }

        /** @brief Returns the number of metadata arguments. */
        std::size_t get_argc() const {
            return this->_args.size();
        }

        /** @brief Returns the metadata argument at the specified position. */
        Expr get_arg_at(std::size_t pos) {
            return this->_args[pos];
        }

        /** @brief Returns an iterator to the first metadata argument. */
        std::vector<Expr>::const_iterator arg_begin() {
            return this->_args.begin();
        }

        /** @brief Returns an iterator past the last metadata argument. */
        std::vector<Expr>::const_iterator arg_end() {
            return this->_args.end();
        }
    };

    /** @brief Represents a function expression.
     *
     * FuncExpr represents an anonymous function definition, including its
     * parameters, optional return type, and optional function body.
     *
     * Function expressions can be used as values and passed or assigned like
     * other expressions.
     */
    class FuncExpr : public IExpr {
        const std::vector<Parameter> _params;
        const Annotation             _return_type = nullptr;
        const Statement              _body        = nullptr;

       public:
        /** @brief Creates an empty function expression. */
        FuncExpr() : _params() {
        }

        /** @brief Creates a function expression with parameters. */
        FuncExpr(std::vector<Parameter>&& args) : _params(std::move(args)) {
        }

        /** @brief Creates a function expression with parameters and return
         * type. */
        FuncExpr(std::vector<Parameter>&& args, Annotation return_type) :
            _params(std::move(args)), _return_type(return_type) {
        }

        /** @brief Creates a complete function expression. */
        FuncExpr(std::vector<Parameter>&& args, Annotation return_type,
                 Statement&& body) :
            _params(std::move(args)), _return_type(return_type),
            _body(std::move(body)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Function;
        }

        /** @brief Returns the number of function parameters. */
        std::size_t get_argc() const {
            return this->_params.size();
        }

        /** @brief Returns the parameter at the specified position. */
        Parameter get_arg_at(std::size_t pos) {
            return this->_params[pos];
        }

        /** @brief Returns an iterator to the first parameter. */
        std::vector<Parameter>::const_iterator arg_begin() {
            return this->_params.begin();
        }

        /** @brief Returns an iterator past the last parameter. */
        std::vector<Parameter>::const_iterator arg_end() {
            return this->_params.end();
        }

        /** @brief Returns the function return type annotation, if available. */
        Annotation get_return_type() {
            return this->_return_type;
        }

        /** @brief Returns the function body, if available. */
        Statement get_body() {
            return this->_body;
        }
    };

    /** @brief Represents a loop expression.
     *
     * LoopExpr represents a conditional loop construct containing optional
     * header expressions, condition lines, and an optional default clause.
     *
     * The header expressions provide values used during loop evaluation, while
     * condition lines define the execution branches for each iteration.
     */
    class LoopExpr : public IExpr {
        std::vector<Expr>          _header;
        std::vector<ConditionLine> _lines;

        // Optional default clause.
        Statement _default = nullptr;

       public:
        /** @brief Creates a loop with condition lines only. */
        LoopExpr(std::vector<ConditionLine>&& lines) :
            _header(), _lines(std::move(lines)) {
        }

        /** @brief Creates a loop with header expressions and condition lines.
         */
        LoopExpr(std::vector<Expr>&&          headers,
                 std::vector<ConditionLine>&& lines) :
            _header(std::move(headers)), _lines(std::move(lines)) {
        }

        /** @brief Creates a loop with headers, condition lines, and a default
         * clause. */
        LoopExpr(std::vector<Expr>&&          headers,
                 std::vector<ConditionLine>&& lines, Statement&& defclause) :
            _header(std::move(headers)), _lines(std::move(lines)),
            _default(std::move(defclause)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Loop;
        }

        /** @brief Returns the number of header expressions. */
        const std::size_t get_headerc() const {
            return this->_header.size();
        }

        /** @brief Returns the number of condition lines. */
        const std::size_t get_condlc() const {
            return this->_lines.size();
        }

        /** @brief Returns a header expression by index. */
        Expr get_header_at(std::size_t pos) const {
            return this->_header[pos];
        }

        /** @brief Returns a condition line by index. */
        ConditionLine get_line_at(std::size_t pos) const {
            return this->_lines[pos];
        }

        /** @brief Returns the default clause, if present. */
        Statement get_default() const {
            return this->_default;
        }
    };

    /** @brief Represents a conditional expression.
     *
     * CondExpr represents a conditional construct containing optional header
     * expressions, condition lines, and an optional default clause.
     *
     * Header expressions provide values used by condition lines, while each
     * condition line defines a branch that is evaluated against those values.
     * The default clause is executed when no condition line matches.
     */
    class CondExpr : public IExpr {
        std::vector<Expr>          _header;
        std::vector<ConditionLine> _lines;

        // Optional default clause.
        Statement _default = nullptr;

       public:
        /** @brief Creates a conditional expression with condition lines only.
         */
        CondExpr(std::vector<ConditionLine>&& lines) :
            _header(), _lines(std::move(lines)) {
        }

        /** @brief Creates a conditional expression with headers and condition
         * lines. */
        CondExpr(std::vector<Expr>&&          headers,
                 std::vector<ConditionLine>&& lines) :
            _header(std::move(headers)), _lines(std::move(lines)) {
        }

        /** @brief Creates a conditional expression with a default clause. */
        CondExpr(std::vector<Expr>&&          headers,
                 std::vector<ConditionLine>&& lines, Statement&& defclause) :
            _header(std::move(headers)), _lines(std::move(lines)),
            _default(std::move(defclause)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Conditional;
        }

        /** @brief Returns the number of header expressions. */
        const std::size_t get_headerc() const {
            return this->_header.size();
        }

        /** @brief Returns the number of condition lines. */
        const std::size_t get_condlc() const {
            return this->_lines.size();
        }

        /** @brief Returns a header expression by index. */
        Expr get_header_at(std::size_t pos) const {
            return this->_header[pos];
        }

        /** @brief Returns a condition line by index. */
        ConditionLine get_line_at(std::size_t pos) const {
            return this->_lines[pos];
        }

        /** @brief Returns the default clause, if present. */
        Statement get_default() const {
            return this->_default;
        }
    };

    /** @brief Represents an allocation expression.
     *
     * AllocExpr represents a memory allocation operation applied to an inner
     * expression.
     *
     * @deprecated don't use we don't have any allocation expression.
     */
    class [[deprecated("There should be no allocation expression.")]] AllocExpr
        : public IExpr {
        const Expr _inner;

       public:
        /** @brief Creates an allocation expression. */
        AllocExpr(Expr&& inner) : _inner(std::move(inner)) {
        }

        /** @brief Returns the expression kind. */
        ExprKind kind() const override {
            return ExprKind::Alloc;
        }

        /** @brief Returns the expression being allocated. */
        Expr get_inner() const {
            return _inner;
        }
    };

#pragma endregion Expressions

}  // namespace Z::Zaban::Langs::ZLang

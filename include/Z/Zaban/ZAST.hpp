#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Z::Zaban {
#pragma region ForwardDeclarations
    class IExpr;
    class IDeclaration;
    class IStatement;
    class ILiteral;
    class IAnnotation;
    class IConditionLine;

    class ParameterBase;
    class VariantFieldBase;

    class CombinatorHalfConditionBase;
#pragma endregion ForwardDeclarations

#pragma region EnumTypes

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
        std::string _name;

        // Optional type annotation.
        Annotation _annotation = nullptr;

        // Optional default initializer.
        Expr _initializer = nullptr;

        // Whether this parameter accepts variadic arguments.
        bool _is_vararg = false;

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

        /** @brief Creates a parameter with a type and initializer expression.
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

        /** @brief Returns whether this parameter accepts variadic arguments. */
        bool is_vararg() const {
            return _is_vararg;
        }

        /** @brief Returns the parameter type annotation, if present. */
        Annotation get_annotation() const {
            return _annotation;
        }

        /** @brief Returns the default initializer expression, if present. */
        Expr get_initializer() const {
            return _initializer;
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

#pragma endregion EnumTypes

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

    /** @brief Shared reference to an expression AST node.
     *
     * Expr provides shared ownership semantics for IExpr nodes. Multiple AST
     * structures may reference the same expression node while the node lifetime
     * is managed automatically through reference counting.
     */
    using Expr = std::shared_ptr<IExpr>;

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

       public:
        /** @brief Creates an identifier annotation with the given name. */
        explicit IdentifierAnnotation(std::string i) : id(std::move(i)) {
        }

        /** @brief Returns the annotation category. */
        AnnotationKind get_kind() const override {
            return AnnotationKind::Base;
        }

        /** @brief Returns the base type category. */
        BaseAnnotationKind get_base_kind() const override {
            return BaseAnnotationKind::Identifier;
        }

        /** @brief Returns the referenced type identifier. */
        std::string get_id() const {
            return id;
        }

        // void accept(Visitors::ASTVisitor* v) override {
        //     v->visit(this);
        // }
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

        // void accept(Visitors::ASTVisitor* v) override {
        //     v->visit(this);
        // }
    };

    /** @brief Represents a struct type annotation with its declared fields. */
    class StructAnnotation : public IBaseAnnotation {
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

        // void accept(Visitors::ASTVisitor* v) override {
        //     v->visit(this);
        // }
    };

    /** @brief Represents a variant type annotation with its possible variants.
     */
    class VariantAnnotation : public IBaseAnnotation {
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
     * TypeDeclaration introduces a named type alias or type binding with an
     * associated annotation.
     *
     * Example:
     * @code
     * type Name : i32;
     * @endcode
     */
    class TypeDeclaration : public IDeclaration {
        const std::string _name;
        const Annotation  _annotation;

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

        /** @brief Returns the associated type annotation. */
        const Annotation get_annotation() const {
            return _annotation;
        }
    };

    /** @brief Represents a variable declaration.
     *
     * LetDeclaration introduces a named value binding. A declaration may
     * optionally contain a type annotation, an initializer expression, or both.
     *
     * Example:
     * @code
     * let value : i32 = 10;
     * @endcode
     */
    class LetDeclaration : public IDeclaration {
        const std::string _name;

        // Optional type annotation.
        const Annotation _type = nullptr;

        // Optional initializer expression.
        const Expr _initializer = nullptr;

       public:
        /** @brief Creates a declaration with only a name. */
        LetDeclaration(std::string name) : _name(name) {
        }

        /** @brief Creates a declaration with an explicit type. */
        LetDeclaration(std::string name, Annotation type) :
            _name(name), _type(std::move(type)), _initializer(nullptr) {
        }

        /** @brief Creates a declaration with an initializer expression. */
        LetDeclaration(std::string name, Expr init) :
            _name(name), _initializer(std::move(init)) {
        }

        /** @brief Creates a declaration with a type and initializer. */
        LetDeclaration(std::string name, Annotation type, Expr init) :
            _name(name), _type(std::move(type)), _initializer(std::move(init)) {
        }

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::LetDecl;
        }

        /** @brief Returns the declared variable name. */
        const std::string get_name() const {
            return _name;
        }

        /** @brief Returns the variable type annotation, if available. */
        const Annotation get_annotation() const {
            return _type;
        }

        /** @brief Returns the initializer expression, if available. */
        const Expr get_initializer() const {
            return _initializer;
        }
    };
#pragma endregion Declartions
}  // namespace Z::Zaban

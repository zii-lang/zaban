# Extended AST

This directory contains the extended AST for ZLang, including the expression tree used to represent program structure and control flow.

The extended AST builds on the base language model by introducing additional node types that support more expressive control-flow constructs and condition evaluation.

## Conditionals

ZLang uses a **sentinel-based structure** for both conditionals and loops. Each construct begins with an opening keyword and ends with a corresponding closing keyword.

Currently, ZLang supports two such constructs:

- **If:** Begins with `if` and ends with `endif`.
- **Loop:** Begins with `loop` and ends with `endloop`.

Both constructs follow the same general structure:

```z
sentinel_start_keyword <header-expressions>

    <condition-line>
    <condition-line>
    ...

sentinel_end_keyword;
```

The header expressions provide the context for evaluating the conditions, while each condition line determines which block of code is executed.

### ConditionLine

A **ConditionLine** represents a conditional branch within an `if` or `loop` construct. Each condition line begins with an operator that defines how it participates in condition evaluation and how its associated block is executed.

The following operators are supported:

- **Chain Operator (`??`)** — Defines a mutually exclusive branch. Conditions are evaluated in order, and once a branch is satisfied, subsequent branches in the same chain are skipped.
- **Serial Operator (`?!`)** — Defines an independent branch that is evaluated in sequence. Each branch is evaluated independently, and its block executes whenever its condition is satisfied, regardless of whether an earlier serial branch matched.
- **Parallel Operator (`!!`)** — Defines an independent branch whose evaluation is not affected by its position in the condition list. Every parallel condition is evaluated, and each matching branch executes its associated block.
- **Else Operator (`::`)** — Defines a fallback branch. Its block executes only when none of the preceding conditions has been satisfied.

#### ConditionLine Examples

##### 1. Chain Conditions

Chain conditions behave like a conventional `if / else if / else` structure. Branches are evaluated in order, and only the first matching branch is executed.

```z
if A, B
    ?? == 20 ?& == 30 => { // Block 1 executes. }
    ?? == 40 ?| == 20 => { // Block 2 executes. }
    :: =>                { // Block 3 executes. }
endif;
```

The example above is equivalent to:

```cpp
if (A == 20 && B == 30) {
    // Block 1 executes.
}
else if (A == 40 || B == 20) {
    // Block 2 executes.
}
else {
    // Block 3 executes.
}
```

##### 2. Serial Conditions

Serial conditions are evaluated independently and sequentially. A later condition is still evaluated even if an earlier condition has already been satisfied.

```z
if A, B
    ?! > 20 ?& > 30 => {}
    ?! > 10 ?& < 40 => {}
endif;
```

This is equivalent to:

```cpp
if (A > 20 && B > 30) {}

if (A > 10 && B < 40) {}
```

Both blocks can execute during the same evaluation if both conditions are satisfied.

##### 3. Parallel Conditions

Parallel conditions are independent of one another. Their evaluation and execution do not depend on the position of other parallel branches in the condition list.

```z
if A, B
    !! > 20 ?& > 40 => {}
    !! > 10 ?& < 10 => {}
endif;
```

Conceptually, this behaves like a collection of independently evaluated conditions:

```text
[
    condition(A > 20 && B > 40),
    condition(A > 10 && B < 10)
].runOnce();
```

Each condition is evaluated independently, and every block whose condition is satisfied is executed.

The distinction between `if` and `loop` becomes particularly important here. An `if` construct evaluates its conditions once, whereas a `loop` repeatedly evaluates its conditions according to the loop's continuation rules.

### ConditionLine Forms

There are three ways to define a condition line in ZLang.

#### 1. Using a Full or Partial Condition

A condition can be expressed in full, or it can be defined relative to an expression provided in the header.

**Full condition:**

```z
if
    ?? a == value => {}
endif;
```

**Partial condition:**

```z
if a
    ?? == value => {}
endif;
```

In the first example, the condition explicitly specifies the expression being tested. In the second, the condition is evaluated relative to the header expression `a`.

#### 2. Using Condition Combinators

Condition combinators allow multiple expressions to be combined into a single condition.

```z
if a, b
    ?? == 20 ?& == 30 => {}
    :: => {}
endif;
```

In this example, `?&` combines the conditions using logical AND, while `::` defines the fallback branch.

#### 3. Using Function Combinators

Function combinators allow a condition to be evaluated through a function call. They provide different ways to select and pass arguments from the condition header to a function.

```z
let check = function(a, b) => {
    return a < b;
};

if a, b
    ?? [] check == true => {}
    ?? [b, a] check == false => {}
    ?? (0, 1) check == false => {}
endif;
```

ZLang supports three types of function condition combinators.

##### Pass All (`[]`)

The **Pass All** combinator passes arguments from the condition header to the function automatically.

```z
if a, b
    ?? [] add == 12 => {}
endif;
```

If the condition header contains `n` arguments and the function accepts `x` arguments, where `x < n`, the first `x` header arguments are passed to the function in their original order.

If the function accepts a variable number of arguments, all `n` header arguments are passed to it.

##### Exact (`[...]`)

The **Exact** combinator lets you specify precisely which arguments are passed to the function.

```z
if a, b
    ?? [a, 12] add == 21 => {}
endif;
```

In this example, the function receives exactly the arguments specified inside the brackets. These arguments can reference expressions from the condition header or any other expression that can be evaluated in the current scope.

##### Positional (`(...)`)

The **Positional** combinator lets you select arguments by their indices in the condition header.

```z
if a, b
    ?? (1, 0) lesser == true => {}
endif;
```

Given a header argument list represented as `array = [a, b]`, the expression `(1, 0)` means the function is called as follows:

```cpp
lesser(array[1], array[0]); // Equivalent to lesser(b, a)
```

This approach allows arguments to be reordered or selected without explicitly naming them.

## Combining Condition Forms

Condition combinators and function combinators can be combined within a single condition line. For example, a function call using the Pass All combinator can be combined with an additional partial condition.

```z
if a, b
    ?? [] sum == 20 ?& == 15 => {}
endif;
```

Conceptually, this is equivalent to:

```cpp
if (sum(a, b) == 20 && a == 15) {}
```

This compositional approach allows function-based conditions, direct comparisons, and logical operators to work together within the same expression.

## LoopExpression

`LoopExpression` represents a looping construct whose condition is not restricted to a single Boolean predicate. Instead, it uses the same condition-combinator infrastructure as other condition-aware expressions in ZLang.

This allows loop conditions to be composed from different logical operations, including:

- Conjunctions
- Disjunctions
- Negations
- Nested condition groups

As a result, a loop's continuation condition can be built from smaller, reusable condition expressions rather than being limited to a single, monolithic Boolean expression.

## IfExpression

`IfExpression` represents the branching counterpart to `LoopExpression`. It evaluates conditions built using the same condition combinators.

Because both constructs share the same condition model, they support a consistent approach to condition evaluation. Conditions can be:

- Combined using logical operators
- Nested to express more complex logic
- Composed from function-based and direct comparisons
- Reused across different control-flow expressions where appropriate

## Design Intent

The shared condition-combinator infrastructure used by `LoopExpression` and `IfExpression` is intended to keep ZLang's control flow consistent, expressive, and extensible.

Rather than treating conditions as isolated Boolean checks, ZLang represents them as composable structures within the AST. This design provides a common foundation for evaluating conditions, combining expressions, and defining branching and looping behavior.

By keeping these concepts consistent at the AST level, the language can support more sophisticated control-flow semantics without requiring an entirely separate condition model for each construct.
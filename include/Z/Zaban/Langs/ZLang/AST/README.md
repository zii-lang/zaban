## Extended AST
This directory contains the ZLang extended AST, including the core expression tree used to model program structure and control flow.
The ZLang AST expands the base language model with additional node types that support more expressive flow-control and condition semantics.

### Conditionals

ZLang uses a **sentinel-based structure** for both conditionals and loops. In this syntax, the opening keyword marks the beginning of the construct, while a corresponding ending keyword marks its termination.

There are currently two types of these constructs:

* **If:** starts with the `if` keyword and ends with `endif`.
* **Loop:** starts with the `loop` keyword and ends with `endloop`.

Both constructs follow the same general structure:

```
sentinel_start_keyword <comma-separated-conditions>

    <condition-line>
    <condition-line>
    ...

sentinel_end_keyword;
```

The conditions specified after the opening keyword determine which condition lines are evaluated or executed.

### ConditionLine

A **ConditionLine** defines a conditional branch within an `if` or `loop` structure. Each condition line begins with an operator that determines how the line participates in condition evaluation.

The following operators are supported:

* **Chain Operator (`??`)** — Defines a mutually exclusive conditional branch. Conditions are evaluated in order, and once a branch is satisfied, subsequent chain branches are not evaluated.
* **Serial Operator (`?!`)** — Defines an independent conditional branch that is evaluated according to its position. Each serial branch is evaluated independently, and its block is executed whenever its condition is satisfied.
* **Parallel Operator (`!!`)** — Defines an independent conditional branch whose evaluation is not affected by the position of other branches. Every parallel condition is evaluated, and its block is executed when the condition is satisfied.
* **Else Operator (`::`)** — Defines the fallback branch. Its block is executed only when no preceding condition has been satisfied.

#### ConditionLine Examples

##### 1. Chain Conditions

Chain conditions behave like a conventional `if / else if / else` structure. Branches are evaluated in order, and only the first matching branch is executed.

```z
if A, B
    ?? == 20 ?& == 30 => { // Block 1 executes. }
    ?? == 40 ?| == 20 => { // Block 2 executes. }
    :: => {                // Block 3 executes. }
endif;
```

The above is equivalent to:

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

Serial conditions are evaluated independently and in sequence. A later condition is still evaluated even if an earlier condition has already been satisfied.

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

Both blocks may therefore execute during the same evaluation.

##### 3. Parallel Conditions

Parallel conditions are independent of one another and are evaluated without regard to their position within the condition list.

```z
if A, B
    !! > 20 ?& > 40 => {}
    !! > 10 ?& < 10 => {}
endif;
```

Conceptually, this behaves like independently scheduled condition blocks:

```text
[
    condition(A > 20 && B > 40),
    condition(A > 10 && B < 10)
].runOnce();
```

Each condition is evaluated independently, and every block whose condition is satisfied is executed.

The distinction becomes particularly important when comparing `if` and `loop` structures. An `if` structure performs a single evaluation of its conditions, whereas a `loop` repeatedly evaluates its conditions and continues executing while the loop's continuation conditions remain satisfied.

### LoopExpression

`LoopExpression` represents a looping construct whose condition is not limited to a simple boolean predicate. Instead, it uses the same condition combinator infrastructure as other ZLang condition-aware expressions.

This allows a loop to be expressed using composed conditions such as:

- conjunctions
- disjunctions
- negations
- nested condition groups

In other words, the loop guard can be built from condition combinators, making the loop condition compositional and consistent with the rest of the language design.

### IfExpression

`IfExpression` is the branching form of the same pattern. It evaluates a condition built from the same condition combinators used by `LoopExpression`.

This means both control-flow structures share an identical condition model, allowing conditions to be:

- combined with logical operators
- nested for more complex evaluation logic
- reused and composed in a uniform way across expressions

## Design intention

The shared use of condition combinators between `LoopExpression` and `IfExpression` ensures that the language's control flow remains consistent and extendable. Conditions are treated as first-class compositional structures rather than as ad hoc boolean checks, which makes the AST more expressive and easier to reason about.

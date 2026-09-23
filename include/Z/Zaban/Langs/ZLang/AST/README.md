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

This directory therefore contains the ZLang extended AST with the following key property:

- `LoopExpression` uses condition combinators
- `IfExpression` uses the same condition combinators

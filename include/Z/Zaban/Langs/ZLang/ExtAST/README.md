# Extended AST

This directory contains the extended AST for ZLang, including the expression tree used to represent program structure and control flow.

The extended AST builds on the base language model by introducing additional node types that support more expressive control-flow constructs and condition evaluation.

## Conditionals

ZLang uses a **sentinel-based structure** for both conditionals and loops. Each construct begins with an opening keyword and ends with a corresponding closing keyword.

Currently, ZLang supports two such constructs:

- **If:** Begins with `if` and ends with `endif`.
- **Loop:** Begins with `loop` and ends with `endloop`.

## Shorthand Condition

Conditional now have a new design, we have shorthands:

```z
<sentinel_start_keyword>
    <expression> "=>" <statement> 
    [ "else" "=>" <statement> ]
<sentinel_end_keyword>
```
It is the simple if-else, loop-else clauses, in shorthand form we do not have else ifs, cause we have when clauses for them, if you need shorthand that much with elseif use another if inside else.

## Condition Header
A list of comma seperated predicates which will be checked within when-else clauses.


## Fullform Condition
Fullform conditionals have multiple clauses each checked with different kind of condition header.
A full form condition is in form of:

```z
<sentinel_start_keyword> <expression> ["," <expression>]*
    <when-clause>+
    ["else" "=>" <statement>]
<sentinel_end_keyword>
```
When clause is described below.

### When-Clause
when clause is like switch cases the form is like:
```z
"when" ["all"] <condition> "=>" <statement>
```
**all** is used whenever you want condition checked independently.
condition can have many forms depending on how the header is defined and how we
want to use them.

### Fullform Condition
This condition comes in full form and does not consume any of the header conditions, an example
of full condition can be without using any header condition:
```z
if
    when a == 12 => { ... }
    when b == 12 => { ... }
    else => { ... }
endif;
```
The example above is equivalent to C/C++ code of:
```c
if(a == 12) {
    ...
} else if (b == 12 ) {
    ...
} else {
    ...
}
```
#### When with "all"
If when clause is used with "all" it is checked independently.
```z
if
    when a == 12 => {...}
    when all b == 12 => {...}
    else => {...}
endif;
```
is equivalent to:
```c
if(a == 12) {

}

if(b == 12) {

}

if(!(a==12) && !(b==12)) {
    // else clause
}
```

### Half Conditions
Half conditions are used to be checked with the header condition, when one is used it discards that
header condition for that specific when clause.

```z
if a
    when == 12 => {...}
    else => {...}
endif;
```
if equivalent to:
```z
if
    when a == 12 => {...}
    else => {...}
endif;
```

### Condition Combinators
With use of `and` and `or` keyword we can combine multiple conditions in our when-clause:
```z
if a, b
    when == 12 and == 13 => {...}
endif;
```
is equivalent to C/C++ code of:
```c
if(a == 12 && b == 13) {...}
```

#### Edge-case of condition combinators
Whenever you use more combinators for half-conditions than headers its a semantic error and compiler will
complain.

```z
if a, b
    when == 12 and == 13 and == 14 => {...}
endif;
```
The above code throws a compiler error.

### Condition Combinator Solution
Maybe you want to check a multiple times in `when` clause, then the consumption of half condition is
not what you want to solve this issue we introduce 3 function-based conditions.

#### Pass All
This kind of function-based conditions passes all of the headers to the function given to it:
```z
let add = func(a : i32, b: i32) {
    return a + b;
};

if a, b
    when [] add == 24 => {...}
endif;
```

you can also use condition combinators since we don't actually consume in this mode:
```z
if a, b
    when [] add == 24 and == 10 => {...}
endif;
```
is equivalent to this C/C++ code:
```c
if( (add(a,b) == 24) && a == 10) {...}
```

When pass all is used if header conditions are more than function acceptable arguments, we pass as much as we can.
But if it's lesser then a compiler error will be thrown.

#### Indexed Functional
In this mode we pass indexes:
```z
if a, b, c
    when (0, 2) some_func == true => {...}
    when (1, 2, 0) some_other_func == false => {...}
endif;
```
in this case above we pass a,c to `some_func` and `b,c,a` in order to `some_other_func`. we can still use half conditions and condition combinators here.

```z
if a, b, c
    when (0, 1) some_func == true and (1, 2) some_func == true => {...}
endif;
```

#### Exact Functional
Well sometimes you need to pass other arguments to the function too, so condition headers are not a thing here.
```z
if a, b, c
    when [10, a] some_func == true => {...}
endif;
```

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

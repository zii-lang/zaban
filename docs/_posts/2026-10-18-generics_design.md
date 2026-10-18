------
layout: post
title: Generics design for Z
------

# Generics Design
Generic programming is about making programming languages more flexible without compromising safety, in programming languages there might be different types and rather than writing functions for each type we can use a generic type and when using types on the go. There are different approaches on how to make it work internally, we will go through each.

## Monomorphization
This way we create specialized functions for each generic.

```
let add<T> : T->T->T = func(A:T, B:T) : T {
    return A + B;
}
```

When it is used with a type like `i32` it is like:

```
add<i32>(12, 30);
```

this will generate a function like:

```llvm
define i32 @add_i32(i32 %A, i32 %B) {
entry:
    %result = add i32 %A, %B
    ret i32 %result
}
```

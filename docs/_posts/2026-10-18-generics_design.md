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

## Dictionary-Passing
When dictionary passing we also might support something like a trait,
like:
```
let add = func<T : Additive>(A: T, B: T) : T {
    return A + B;
}
```
Now a type like `i8` should be : Addtive to another `i8` type if they are used.

**Experimental:**
```
trait Additive = (A, B) {
    A + B
}
```
#if target=llvm
type i8 = abstract {
    trait Additive {
        Transitive @:op(A : i8, B: i8) {
            %result = %i8 add %A, %B
            return %result;
        }
    }
}
#endif

## Valid Design
Well we should specify where we are able to use generics like:
```
let add<T> = func<T>(a:T, b:T) : T {
    return a + b;
}
```
The redundency of use of `<T>` in LHS and RHS, is visible so which side should it be?
anonymous functions can also have generics so LHS should inherit the function of RHS.
So this function should be written like:
```
let add = func<T>(a:T, b:T) : T {
    return a + b;
}
```
but what about explicit type declaration?
well in case of generics we don't have explicit type, it is always implicit type.
if we use it like this then it is possible:
```
let i32_add : i32->i32->i32 = add<i32>;
```

in the case above if you use this function the generated generic function will actually have
the name you defined.

### Which expression should be generic?
In Z when we can declare:
1- function expressions
2- structures
3- variants
4- enums
With generics:

```
type point = struct<T> {
    x : T,
    y : T,
    z : T,
};

type x = enum<T> : T {
    A,
    B,
    C,
};

type color = vari<T> : i32 {
    Red = 0xff0000,
    Green = 0x00ff00,
    Blue = 0x0000ff,
    RGB(r: T, g: T, b: T),
};
```

We can give default generic values too:

```
type color = vari<T = i32> : i32 {
    ...
}
```
in this case if color is used without `<T>` then i32 type is used.
# Chapter 3 — Basic Syntax & Variables

## Comments

MRT supports single-line and multi-line block comments:

```mrt
// Single line comment
/*
   Multi-line
   block comment
*/
```

## Variable Declarations (`var`)

Variables are declared using the `var` keyword:

```mrt
var name = "Murat"
var age = 30
var pi = 3.14159
var active = yes
var inactive = no
var nothing = none
```

## Data Types

- **Integers (`int`)**: 64-bit signed integers (`42`, `-10`).
- **Floating-point (`float`)**: 64-bit IEEE-754 numbers (`3.14`, `-0.05`).
- **Booleans (`bool`)**: `yes` (true) and `no` (false).
- **Strings (`string`)**: UTF-8 encoded text enclosed in double quotes (`"Hello world"`).
- **None (`none`)**: Represents absence of value.

## Operators

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Logical**: `and`, `or`, `not`
- **Concatenation**: `+` concatenates strings and arrays.

```mrt
var full_name = "Murat " + "Özçelik"
var numbers = [1, 2] + [3, 4]
```

## Printing Output (`say`)

The `say` statement prints an expression's value followed by a newline:

```mrt
say "Hello MRT!"
say 100 + 250
```

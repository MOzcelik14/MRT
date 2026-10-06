# MRT Language Reference

## Identifiers & Keywords

### Keywords
`let`, `fn`, `return`, `if`, `else`, `while`, `and`, `or`, `not`, `true`, `false`, `null`

### Comments
Single-line comments begin with `//` and extend to the end of the line:
```mrt
// This is a comment
let x = 10
```

## Operators & Precedence

Precedence levels from lowest to highest:

1. **Assignment**: `=` (right-associative)
2. **Logical OR**: `or` (left-associative)
3. **Logical AND**: `and` (left-associative)
4. **Equality**: `==`, `!=` (left-associative)
5. **Comparison**: `<`, `<=`, `>`, `>=` (left-associative)
6. **Term**: `+`, `-` (left-associative)
7. **Factor**: `*`, `/`, `%` (left-associative)
8. **Unary**: `-`, `not` (prefix)
9. **Call**: `()` (postfix)
10. **Primary**: Literals, Identifiers, Grouped `(expr)`

## Built-in Functions

- `print(...)`: Prints arguments to stdout.
- `typeof(val)`: Returns `"integer"`, `"float"`, `"string"`, `"boolean"`, `"null"`, or `"function"`.
- `len(str)`: Returns integer character count.
- `str(val)`: Converts any value into a string.
- `clock()`: Returns CPU time in seconds.

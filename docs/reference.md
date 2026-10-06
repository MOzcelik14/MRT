# MRT Language Reference (v0.2.0)

## Identifiers & Keywords

### Keywords
`var`, `task`, `give`, `say`, `when`, `otherwise`, `repeat`, `break`, `continue`, `and`, `or`, `not`, `yes`, `no`, `none`

### Comments
Single-line comments begin with `//`:
```mrt
// This is a single-line comment
var x = 10
```

Multi-line block comments begin with `/*` and end with `*/`:
```mrt
/*
   This is a multi-line
   block comment
*/
var name = "Murat"
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

## Built-in Functions & Statements

- `say <expr>`: Output statement that evaluates an expression and prints it to stdout followed by a newline.
- `typeOf(val)`: Returns `"integer"`, `"float"`, `"string"`, `"boolean"`, `"none"`, or `"function"`.
- `length(str)`: Returns the integer character count of a string.
- `toText(val)`: Converts any value into a string representation.
- `clock()`: Returns process execution time in seconds as a float.

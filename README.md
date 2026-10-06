# MRT

MRT is a small programming language implemented from scratch in C.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](#building)
[![Language](https://img.shields.io/badge/language-C11%20%2F%20C17-blue)](#building)
[![Standard](https://img.shields.io/badge/standard%20library-only-orange)](#overview)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

---

## Overview

**MRT** is a modern, clean, dynamically typed scripting language designed and implemented entirely from scratch in standard C (C11/C17). It uses zero third-party dependencies, parsers, or lexer generators (no Flex or Bison).

All source files in MRT use the **`.mrt`** extension:

```text
main.mrt
hello.mrt
factorial.mrt
```

The executable name is `mrt`.

---

## Features

- **Standard C (C11/C17)**: Fully portable implementation using only the C standard library.
- **Hand-written Frontend**: Recursive descent parser with precedence climbing and clean lexical analysis.
- **Decoupled Architecture**: Strict separation between the Frontend (Lexer, Parser, AST) and Runtime (Interpreter, Environment, Value system). Ready for future Bytecode VM and C transpile backends.
- **Dynamic Typing**: Native support for `integer`, `float`, `string`, `boolean`, `null`, and `function`.
- **First-Class Functions & Recursion**: Function declarations (`fn`), closures, nested scopes, parameter binding, and tail/nested recursion.
- **Lexical Scoping**: Deterministic environment hierarchy with parent pointer chains.
- **Quality Error Diagnostics**: GCC/Rust-style source code context with line numbers, column numbers, and visual caret (`^`) pointers.
- **Interactive REPL**: Interactive shell with statement and persistent expression evaluation.
- **Debug Inspection**: Built-in CLI flags to inspect token streams (`--tokens`) and abstract syntax trees (`--ast`).
- **Memory Safety**: Deterministic reference-counted object management verified under AddressSanitizer and LeakSanitizer for **0 bytes memory leaks**.

---

## Architecture

The project is structured with an extensible pipeline:

```text
MRT Source (.mrt)
        ↓
    [ Lexer ]
        ↓ Tokens
    [ Parser ]
        ↓ AST  <--- Decoupled Frontend Boundary
  [ AST Interpreter ]
        ↓
     Output
```

The AST representation is independent of runtime details. Future releases will plug in:
1. **Bytecode Compiler & VM**: `AST -> Bytecode -> MRT VM`
2. **C Backend**: `AST -> Generated C -> GCC / Clang -> Native Binary`

---

## Project Structure

```text
mrt/
├── src/
│   ├── common.h         # Memory wrappers and common utilities
│   ├── version.h        # Central version definition (MRT 0.1.0)
│   ├── token.h          # Token types and structures
│   ├── token.c          # Token printers and constructors
│   ├── error.h          # Diagnostic error reporting interface
│   ├── error.c          # Caret positioning and error formatting
│   ├── lexer.h          # Hand-crafted lexer interface
│   ├── lexer.c          # Scanner for MRT grammar and literals
│   ├── ast.h            # Abstract Syntax Tree nodes and definitions
│   ├── ast.c            # AST construction, freeing, and tree printer
│   ├── parser.h         # Recursive-descent parser interface
│   ├── parser.c         # Operator precedence parser & sync recovery
│   ├── value.h          # Tagged union Value system & string management
│   ├── value.c          # Value operations and memory retain/release
│   ├── environment.h    # Lexical scope symbol table interface
│   ├── environment.c    # Hash table scope implementation
│   ├── builtin.h        # Standard builtins (print, typeof, len, str)
│   ├── builtin.c        # Builtin function implementations
│   ├── interpreter.h    # AST tree-walking interpreter
│   ├── interpreter.c    # Evaluation engine & expression evaluation
│   └── main.c           # CLI entry point, argument parsing & REPL
├── tests/
│   └── test_main.c      # Automated test runner and test cases
├── examples/
│   ├── hello.mrt        # Basic printing example
│   ├── variables.mrt    # Types and variable declarations
│   ├── conditions.mrt   # If / else branches and comparisons
│   ├── loops.mrt        # While loop example
│   ├── functions.mrt    # Function definitions and scoping
│   └── factorial.mrt    # Recursive factorial calculation
├── docs/
│   ├── architecture.md  # Architectural overview and design principles
│   └── reference.md     # Language syntax and builtin reference
├── Makefile             # Build automation
├── README.md            # Project documentation
└── LICENSE              # MIT License
```

---

## Building

### Requirements
- GCC (11+) or Clang (13+)
- GNU Make
- Linux (Ubuntu, Debian, Fedora, Arch, etc.)

### Build Targets

Build release binary with `-O2`:
```bash
make
```

Build debug binary with AddressSanitizer and UndefinedBehaviorSanitizer:
```bash
make debug
```

Run test suite:
```bash
make test
```

Run sample program:
```bash
make run
```

Clean build artifacts:
```bash
make clean
```

---

## Usage

### Run a `.mrt` File

```bash
./mrt examples/hello.mrt
```

### Interactive REPL

Start the REPL by running `mrt` without arguments:

```bash
$ ./mrt
MRT 0.1.0
Interactive interpreter

>>> let x = 10
>>> x + 5
15
>>> print("Merhaba")
Merhaba
```

### CLI Options

Display version:
```bash
./mrt --version
# MRT 0.1.0
```

Display help:
```bash
./mrt --help
```

Dump Token Stream:
```bash
./mrt --tokens examples/factorial.mrt
```

Dump Abstract Syntax Tree:
```bash
./mrt --ast examples/factorial.mrt
```

Example AST output:
```text
Program
├── FunctionDecl(factorial(n))
│   └── Block
│       ├── IfStmt
│       │   ├── BinaryExpr(<=)
│       │   │   ├── Identifier(n)
│       │   │   └── Integer(1)
│       │   └── Block
│       │       └── ReturnStmt
│       │           └── Integer(1)
│       └── ReturnStmt
│           └── BinaryExpr(*)
│               ├── Identifier(n)
│               └── CallExpr
│                   ├── Identifier(factorial)
│                   └── BinaryExpr(-)
│                       ├── Identifier(n)
│                       └── Integer(1)
└── ExprStmt
    └── CallExpr
        ├── Identifier(print)
        └── CallExpr
            ├── Identifier(factorial)
            └── Integer(5)
```

---

## Language Syntax

### Data Types

| Type | Examples | Description |
|---|---|---|
| `integer` | `10`, `-42`, `0` | 64-bit signed integer |
| `float` | `3.14`, `-0.5` | Double-precision floating point |
| `string` | `"Murat"`, `"Selam\n"` | Character sequence with escape sequences (`\n`, `\t`, `\"`, `\\`) |
| `boolean` | `true`, `false` | Boolean truth values |
| `null` | `null` | Absence of value |
| `function`| `fn (a, b) { ... }`| First-class callable routines |

### Variables & Assignment

Variables are defined with `let` and can be assigned new values:

```mrt
let x = 10
x = 20
```

Statement terminators can be newlines or semicolons:
```mrt
let a = 1; let b = 2;
```

### Arithmetic Operators

Supports standard precedence (`*`, `/`, `%` bind tighter than `+`, `-`):

```mrt
let x = 10 + 5 * 2   // Evaluates to 20
let y = (10 + 5) * 2 // Evaluates to 30
let m = 14 % 4       // Evaluates to 2
```

### Comparisons

```mrt
==    !=    <    <=    >    >=
```

### Boolean Logic

Keywords `and`, `or`, and `not` with short-circuit evaluation:

```mrt
if yas >= 18 and aktif {
    print("Giriş izni var")
}
```

### String Concatenation

Use `+` to concatenate strings:

```mrt
let isim = "Murat"
print("Merhaba " + isim)
```

### Control Flow

#### If / Else

```mrt
if yas >= 18 {
    print("Yetişkin")
} else {
    print("Çocuk")
}
```

Nested and `else if` chains are fully supported:
```mrt
if puan >= 90 {
    print("A")
} else if puan >= 80 {
    print("B")
} else {
    print("C")
}
```

#### While Loops

```mrt
let i = 0
while i < 10 {
    print(i)
    i = i + 1
}
```

### Functions & Scoping

Functions are declared with `fn` and support lexical scoping, local variables, parameters, and recursion:

```mrt
fn topla(a, b) {
    return a + b
}

let sonuc = topla(10, 20)
print(sonuc)
```

Recursion:

```mrt
fn faktoriyel(n) {
    if n <= 1 {
        return 1
    }
    return n * faktoriyel(n - 1)
}

print(faktoriyel(5)) // Prints 120
```

### Lexical Scope

```mrt
let x = 10

fn test() {
    let x = 20
    print(x) // Prints 20
}

test()
print(x)     // Prints 10
```

### Built-in Functions

- `print(...)`: Prints values separated by spaces to standard output.
- `typeof(x)`: Returns the type name as string (`"integer"`, `"float"`, `"string"`, `"boolean"`, `"null"`, `"function"`).
- `len(x)`: Returns the length of a string.
- `str(x)`: Converts a value to its string representation.
- `clock()`: Returns elapsed execution time in seconds.

---

## Testing

The project includes an automated test runner in `tests/test_main.c`:

```bash
make test
```

Tests cover:
- Lexer tokenization & escapes
- Numbers (int & float)
- Strings & concatenation
- Operators & precedence climbing
- Variable declarations & assignments
- Boolean logic & short-circuit evaluation
- Lexical scoping & variable shadowing
- Functions, arguments & returns
- Deep recursion (Factorial, Fibonacci)
- Control flow (`if`, `else`, `while`)
- Builtin functions
- Error handling (`SyntaxError`, `NameError`, `TypeError`, `RuntimeError`)
- Memory leak detection via AddressSanitizer and LeakSanitizer

---

## Roadmap

### MRT 0.2
- [ ] Arrays and dynamic lists (`[1, 2, 3]`)
- [ ] Maps / Dictionaries (`{ "key": value }`)
- [ ] `for` loops (`for item in list`)
- [ ] `break` and `continue` statements
- [ ] Import and module system (`import math`)

### MRT 0.3
- [ ] Bytecode compiler:
  ```text
  MRT Source -> Lexer -> Parser -> AST -> Bytecode Compiler -> MRT VM
  ```
- [ ] Stack-based Virtual Machine
- [ ] Disassembler tools (`--disasm`)

### MRT 0.4
- [ ] C Backend:
  ```text
  MRT -> AST -> Generated C -> GCC / Clang -> Native Binary
  ```
- [ ] Standalone native compilation

### Future
- [ ] Mark-and-sweep Garbage Collector
- [ ] Standard library extensions (Filesystem, Math, Regex, Net)
- [ ] Language Server Protocol (LSP) implementation
- [ ] Source formatter & Syntax highlighter
- [ ] Interactive source debugger

---

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/my-feature`).
3. Ensure all tests pass under `make test` with zero warnings and zero memory leaks.
4. Commit your changes (`git commit -am 'Add new feature'`).
5. Push to the branch (`git push origin feature/my-feature`).
6. Create a new Pull Request.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

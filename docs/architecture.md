# MRT Architecture Guide

## 1. Design Philosophy

MRT is designed with modularity, decoupling, and zero external dependencies in mind. Every stage of the compiler pipeline is decoupled from adjacent stages through clear abstract data structures.

```
       Source File (.mrt)
               │
               ▼
       ┌───────────────┐
       │     Lexer     │
       └───────┬───────┘
               │ Tokens
               ▼
       ┌───────────────┐
       │    Parser     │
       └───────┬───────┘
               │ Abstract Syntax Tree (AST)
       ────────┴───────────────────────────  (Frontend Boundary)
               │
               ├────────────────────────┬────────────────────────┐
               ▼                        ▼                        ▼
       ┌───────────────┐        ┌───────────────┐        ┌───────────────┐
       │AST Interpreter│        │Bytecode (0.3) │        │ C Transpiler  │
       │   (MRT 0.1)   │        │     & VM      │        │    (MRT 0.4)  │
       └───────────────┘        └───────────────┘        └───────────────┘
```

## 2. Frontend Decoupling

The frontend consists of:
- `token.c` / `token.h`: Pure token definitions.
- `lexer.c` / `lexer.h`: Scans raw characters into token streams.
- `ast.c` / `ast.h`: Pure syntax trees without runtime state.
- `parser.c` / `parser.h`: Converts tokens into ASTs using recursive descent.

Neither the Lexer nor the Parser has any knowledge of `Value`, `Environment`, or evaluation logic. This ensures the AST can be repurposed for other backends without any modifications.

## 3. Runtime & Memory Management

The runtime consists of:
- `value.c` / `value.h`: Tagged union `Value` representing runtime data types (`integer`, `float`, `string`, `boolean`, `null`, `function`, `native_fn`).
- `environment.c` / `environment.h`: Lexical scope environments organized in an acyclic parent chain (`enclosing`).
- `builtin.c` / `builtin.h`: Native functions bound into the root environment.
- `interpreter.c` / `interpreter.h`: Recursive AST visitor that performs dynamic evaluation.

### Deterministic Reference Counting
- Strings (`MrtString`) and functions (`MrtFunction`) carry a `ref_count`.
- Scope environments (`Environment`) carry a `ref_count`.
- Intermediate evaluation values are freed immediately upon use.
- The runtime is verified under AddressSanitizer with zero memory leaks.

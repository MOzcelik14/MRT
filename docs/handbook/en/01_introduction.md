# Chapter 1 — Introduction to MRT

## What is MRT?

**MRT** is a lightweight, modern, readable, and extensible general-purpose programming language implemented from scratch in C11. It does not rely on parser generators (Lex, Yacc, Flex, Bison) or third-party runtime dependencies.

With version **0.2.0**, MRT has established its distinctive language identity, enriched its data structures with first-class Collections (Arrays and Maps), introduced a module system (`use`), robust CLI toolchains (`mrt check`, `mrt fmt`, `mrt inspect`), and a native GTK4 integrated development environment: **MRT Studio 0.2**.

## Core Design Principles

1. **Recognizable Language Identity**: Uses distinct keywords (`var`, `task`, `give`, `when`, `otherwise`, `repeat`, `each ... in ...`, `say`) rather than imitating other languages.
2. **Lightweight & Self-Contained**: Requires only a standard C compiler (GCC or Clang) and the C standard library.
3. **Memory Safety**: Powered by reference-counted memory management that passes strict AddressSanitizer and UndefinedBehaviorSanitizer checks with zero leaks.
4. **Cohesive Tooling**: CLI tools for running, checking (`check`), formatting (`fmt`), and symbol inspection (`inspect`) operate identically from the terminal and within MRT Studio.
5. **Linux-First**: Integrates seamlessly with modern Linux desktop environments.

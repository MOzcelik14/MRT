# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.2.0] - 2026-10-07

### Added
- **Collections**:
  - Dynamically sized ref-counted Arrays (`[1, 2, 3]`) with index access `arr[i]` and mutation `arr[i] = val`.
  - Array concatenation operator (`[1, 2] + [3, 4]`).
  - Hash Maps (`{"key": value}`) with key indexing `map["key"]` and mutation `map["key"] = val`.
- **Control Flow**:
  - `each ... in ...` loops over arrays, maps, and strings with full `break` and `continue` support.
- **Module System**:
  - `use "path/file.mrt"` module import statement with relative path resolution, circular import detection, and single-execution caching.
- **Standard Library Additions**:
  - `append(array, item)`: Appends element to array.
  - `remove(collection, key_or_index)`: Removes element by index or key and returns it.
  - `contains(collection, item_or_key)`: Checks membership in array, map, or string.
  - `keys(map)`: Returns array of string keys.
  - `values(map)`: Returns array of values.
  - `range(start, end[, step])`: Generates integer array range.
  - `number(value)` (alias `toNumber`): Converts string/boolean/int/float to number.
  - `assert(condition[, message])`: Raises RuntimeError if condition evaluates to falsy.
  - Multi-type `length()`: Now supports strings, arrays, and maps.
  - Preserved existing interactive input function: `read(prompt)`.
- **Runtime Diagnostics**:
  - Call stack frames (`CallFrame`) with function traces `at func(file:line:col)`.
- **CLI Subcommands**:
  - `mrt run <file.mrt>`: Execute script.
  - `mrt check [--diagnostics=json] <file.mrt>`: Fast syntax check with human or JSON diagnostic reporting.
  - `mrt fmt [--check] <file.mrt>`: AST-based source code formatter with in-place formatting or diff checking.
  - `mrt inspect --symbols [--json] <file.mrt>`: AST symbol inspector producing human-readable outline or structured JSON.
  - `mrt repl`: Interactive Read-Eval-Print Loop.
  - `mrt version` & `mrt help`: Subcommands for information.
- **MRT Studio 0.2**:
  - Document formatting support via `mrt fmt` bound to `Shift+Alt+F` and `Edit -> Format Document`.
  - Updated GtkSourceView 5 language definition `data/mrt.lang` with all MRT 0.2 keywords and built-in functions.
  - Autocompletion word buffer updated with MRT 0.2 grammar and builtins.
  - Bilingual UI enhancements (Turkish and English).
- **Tooling & Infrastructure**:
  - Dual build system support: `Makefile` and `Meson + Ninja` (`meson.build`, `meson_options.txt`).
  - `.clang-format` configuration.
  - GitHub Actions CI workflow (`.github/workflows/ci.yml`) testing GCC and Clang.
  - Bilingual Handbook 2.0 (`docs/handbook/tr/` and `docs/handbook/en/`).
  - `AI_ASSISTED.md` transparency disclosure.

### Changed
- Keyword evolution from 0.1 to strengthen identity: `var`, `task`, `give`, `when`, `otherwise`, `repeat`, `say`, `yes`, `no`, `none`.
- Top-level `give` cleanly returns from the program with status OK.

---

## [0.1.0] - 2026-10-06

### Added
- Initial release of the MRT programming language.
- Handcrafted recursive-descent lexer and parser in C11.
- Tree-walk AST interpreter.
- Core types: integers, floats, booleans, strings, functions, none.
- Basic builtins: `say`, `read`, `clock`, `typeOf`, `toText`.
- Initial version of MRT Studio with GTK4 and GtkSourceView 5.

# Chapter 2 — Installation & Tooling

## Building and Installing

MRT supports both `Makefile` and `Meson + Ninja` build workflows.

### Building with Makefile

```bash
# Build compiler and CLI
make

# Run test suite with AddressSanitizer
make test

# Build MRT Studio IDE
make studio

# Install to system
make install PREFIX=/usr/local
```

### Building with Meson & Ninja

```bash
meson setup build
meson compile -C build
meson test -C build
```

## CLI Subcommands

MRT 0.2.0 includes a comprehensive CLI toolchain:

### 1. `mrt run <file.mrt>`
Executes an MRT source file. Running `mrt <file.mrt>` also runs by default.
```bash
mrt run main.mrt
```

### 2. `mrt check [--diagnostics=json] <file.mrt>`
Validates syntax and AST structure without executing.
- Human mode: Prints source context and carets pointing to errors.
- JSON mode (`--diagnostics=json`): Outputs structured diagnostic JSON for IDEs and CI bots.
```bash
mrt check --diagnostics=json main.mrt
```

### 3. `mrt fmt [--check] <file.mrt>`
AST-based source code formatter.
- `--check`: Verifies whether a file conforms to formatting rules without modifying it.
```bash
mrt fmt main.mrt
mrt fmt --check main.mrt
```

### 4. `mrt inspect --symbols [--json] <file.mrt>`
Inspects AST declarations (tasks, variables) with line and column numbers.
```bash
mrt inspect --symbols main.mrt
mrt inspect --symbols --json main.mrt
```

### 5. `mrt repl`
Starts the interactive interpreter REPL.
```bash
mrt repl
```

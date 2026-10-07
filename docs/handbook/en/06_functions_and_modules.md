# Chapter 6 — Functions & Modules

## Defining Functions (`task`)

Functions are first-class citizens declared with the `task` keyword:

```mrt
task add(a, b) {
    give a + b
}

var result = add(15, 25)
say "Result: " + toText(result)
```

## Returning Values (`give`)

The `give` statement terminates function execution and returns a value to the caller:

```mrt
task absolute(x) {
    when x < 0 {
        give -x
    }
    give x
}
```

If a function ends without an explicit `give`, it implicitly returns `none`.

---

## The Module System (`use`)

MRT 0.2.0 introduces clean file modularity using `use`:

### Defining a Module (`math.mrt`)
```mrt
// math.mrt
task square(x) {
    give x * x
}

task multiply(a, b) {
    give a * b
}

var pi = 3.14159
```

### Importing a Module (`main.mrt`)
```mrt
// main.mrt
use "math.mrt"

say "Pi: " + toText(pi)
say "Square of 6: " + toText(square(6))
say "Multiplication: " + toText(multiply(5, 8))
```

### Module Features
- **Relative Resolution**: Paths in `use` are resolved relative to the calling source file.
- **Single-Execution Caching**: Modules are executed only once regardless of how many times they are imported.
- **Circular Dependency Detection**: Prevents infinite recursion if modules mutually import each other.

# Chapter 5 — Control Flow

## Conditionals (`when` / `otherwise`)

Branching in MRT is controlled by `when` and `otherwise`:

```mrt
var score = 85

when score >= 90 {
    say "Grade: A"
} otherwise when score >= 80 {
    say "Grade: B"
} otherwise {
    say "Grade: C"
}
```

## Loops

### 1. While Loops (`repeat`)
Repeats the body as long as the condition evaluates to `yes`:

```mrt
var counter = 5
repeat counter > 0 {
    say "Countdown: " + toText(counter)
    counter = counter - 1
}
```

### 2. For-Each Loops (`each ... in ...`)
Iterates over collections (arrays, maps, strings, and ranges):

```mrt
// Array iteration
each item in ["apple", "banana", "orange"] {
    say "Fruit: " + item
}

// Range iteration
each n in range(1, 5) {
    say "Step: " + toText(n)
}

// Map iteration
var config = {"env": "prod", "debug": "no"}
each key in keys(config) {
    say key + " = " + config[key]
}
```

## Loop Controls (`break` / `continue`)

- `break`: Terminates the nearest enclosing loop immediately.
- `continue`: Advances to the next loop iteration.

```mrt
each n in range(1, 20) {
    when n == 5 {
        continue // skip 5
    }
    when n == 10 {
        break    // exit at 10
    }
    say n
}
```

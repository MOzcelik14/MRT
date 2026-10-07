# Chapter 7 — Standard Library Built-ins

MRT provides the following built-in functions in the global environment:

| Function | Parameters | Description |
|---|---|---|
| `read([prompt])` | `prompt` (optional string) | Reads input from standard input as a string. |
| `number(value)` | `value` | Converts string/bool/number to integer or float. Alias: `toNumber`. |
| `toText(value)` | `value` | Converts any MRT value to its string representation. |
| `typeOf(value)` | `value` | Returns type name as string (`"int"`, `"float"`, `"string"`, `"bool"`, `"array"`, `"map"`, `"function"`, `"none"`). |
| `length(coll)` | `string`, `array`, or `map` | Returns number of elements or characters. |
| `append(array, item)` | `array`, `item` | Appends item to array in place. |
| `remove(coll, key_idx)` | `array` + `index` or `map` + `key` | Removes item by index or key and returns it. |
| `contains(coll, item)` | `coll` (array, map, string), `item` | Returns `yes` if present, else `no`. |
| `keys(map)` | `map` | Returns array of string keys. |
| `values(map)` | `map` | Returns array of values. |
| `range(start, end[, step])` | `int`, `int`[, `int`] | Produces an integer array range. |
| `assert(cond[, msg])` | `condition`, `message` | Throws RuntimeError if condition is falsy. Essential for testing. |
| `clock()` | none | Returns elapsed CPU time in seconds as float. |

## Examples

```mrt
// User input & validation
var inp = read("Enter age: ")
var age = number(inp)
assert(age > 0, "Age must be positive")

// Collections
var list = range(1, 5) // [1, 2, 3, 4]
append(list, 100)
say "Length: " + toText(length(list))
say "Has 100? " + toText(contains(list, 100))
```

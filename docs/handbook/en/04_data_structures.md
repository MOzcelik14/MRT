# Chapter 4 — Collections & Data Structures

MRT 0.2.0 features two first-class collection types: **Arrays** and **Maps**.

## 1. Arrays

Arrays are dynamically sized, zero-indexed ordered lists denoted by square brackets `[...]`:

```mrt
var items = [10, 20, 30]
var empty = []
```

### Element Access and Mutation
```mrt
var first = items[0]    // 10
items[1] = 99           // Updates second element
say items               // [10, 99, 30]
```

### Array Operations
- `append(array, item)`: Adds an item to the end of the array.
- `remove(array, index)`: Removes and returns the element at the index.
- `contains(array, item)`: Checks whether the item exists in the array.
- `length(array)`: Returns the number of items.
- `+`: Concatenates two arrays into a new array.

---

## 2. Maps

Maps are key-value associative dictionaries written with curly braces `{ ... }`, where keys are strings:

```mrt
var user = {
    "name": "Murat",
    "role": "Architect",
    "version": "0.2.0"
}
```

### Map Access and Mutation
```mrt
say user["name"]           // Murat
user["role"] = "Lead"     // Mutation
user["city"] = "Ankara"    // Insertion
```

### Map Operations
- `keys(map)`: Returns an array of keys.
- `values(map)`: Returns an array of values.
- `contains(map, key)`: Returns `yes` if the key exists, else `no`.
- `remove(map, key)`: Removes the entry and returns its value.
- `length(map)`: Returns the count of key-value pairs.

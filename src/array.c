#include "array.h"
#include "value.h"

MrtArray *mrt_array_new(void) {
    return mrt_array_with_capacity(8);
}

MrtArray *mrt_array_with_capacity(size_t capacity) {
    MrtArray *arr = (MrtArray *)mrt_malloc(sizeof(MrtArray));
    arr->ref_count = 1;
    arr->count = 0;
    arr->capacity = capacity > 0 ? capacity : 4;
    arr->elements = (Value *)mrt_malloc(sizeof(Value) * arr->capacity);
    return arr;
}

void mrt_array_retain(MrtArray *arr) {
    if (arr) {
        arr->ref_count++;
    }
}

void mrt_array_release(MrtArray *arr) {
    if (!arr) return;
    arr->ref_count--;
    if (arr->ref_count <= 0) {
        for (size_t i = 0; i < arr->count; i++) {
            value_release(arr->elements[i]);
        }
        mrt_free(arr->elements);
        mrt_free(arr);
    }
}

void mrt_array_push(MrtArray *arr, Value val) {
    if (!arr) return;
    if (arr->count >= arr->capacity) {
        arr->capacity = arr->capacity * 2;
        arr->elements = (Value *)mrt_realloc(arr->elements, sizeof(Value) * arr->capacity);
    }
    value_retain(val);
    arr->elements[arr->count++] = val;
}

Value mrt_array_get(MrtArray *arr, size_t index) {
    if (!arr || index >= arr->count) {
        return value_none();
    }
    return arr->elements[index];
}

void mrt_array_set(MrtArray *arr, size_t index, Value val) {
    if (!arr || index >= arr->count) return;
    value_retain(val);
    value_release(arr->elements[index]);
    arr->elements[index] = val;
}

bool mrt_array_remove_at(MrtArray *arr, size_t index, Value *out_removed) {
    if (!arr || index >= arr->count) return false;
    if (out_removed) {
        *out_removed = arr->elements[index];
    } else {
        value_release(arr->elements[index]);
    }
    for (size_t i = index; i + 1 < arr->count; i++) {
        arr->elements[i] = arr->elements[i + 1];
    }
    arr->count--;
    return true;
}

#ifndef MRT_ARRAY_H
#define MRT_ARRAY_H

#include "common.h"
#include "value.h"

typedef struct MrtArray {
    int ref_count;
    struct Value *elements;
    size_t count;
    size_t capacity;
} MrtArray;

MrtArray *mrt_array_new(void);
MrtArray *mrt_array_with_capacity(size_t capacity);
void mrt_array_retain(MrtArray *arr);
void mrt_array_release(MrtArray *arr);

void mrt_array_push(MrtArray *arr, struct Value val);
struct Value mrt_array_get(MrtArray *arr, size_t index);
void mrt_array_set(MrtArray *arr, size_t index, struct Value val);
bool mrt_array_remove_at(MrtArray *arr, size_t index, struct Value *out_removed);

#endif /* MRT_ARRAY_H */

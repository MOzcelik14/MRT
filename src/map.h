#ifndef MRT_MAP_H
#define MRT_MAP_H

#include "common.h"
#include "value.h"

typedef struct MrtMapEntry {
    char *key;
    struct Value value;
} MrtMapEntry;

typedef struct MrtMap {
    int ref_count;
    MrtMapEntry *entries;
    size_t count;
    size_t capacity;
} MrtMap;

MrtMap *mrt_map_new(void);
void mrt_map_retain(MrtMap *map);
void mrt_map_release(MrtMap *map);

bool mrt_map_get(MrtMap *map, const char *key, struct Value *out_val);
void mrt_map_set(MrtMap *map, const char *key, struct Value val);
bool mrt_map_remove(MrtMap *map, const char *key, struct Value *out_removed);
bool mrt_map_has(MrtMap *map, const char *key);

#endif /* MRT_MAP_H */

#include "map.h"
#include "value.h"

MrtMap *mrt_map_new(void) {
    MrtMap *map = (MrtMap *)mrt_malloc(sizeof(MrtMap));
    map->ref_count = 1;
    map->count = 0;
    map->capacity = 4;
    map->entries = (MrtMapEntry *)mrt_malloc(sizeof(MrtMapEntry) * map->capacity);
    return map;
}

void mrt_map_retain(MrtMap *map) {
    if (map) {
        map->ref_count++;
    }
}

void mrt_map_release(MrtMap *map) {
    if (!map) return;
    map->ref_count--;
    if (map->ref_count <= 0) {
        for (size_t i = 0; i < map->count; i++) {
            mrt_free(map->entries[i].key);
            value_release(map->entries[i].value);
        }
        mrt_free(map->entries);
        mrt_free(map);
    }
}

bool mrt_map_get(MrtMap *map, const char *key, Value *out_val) {
    if (!map || !key) return false;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            if (out_val) *out_val = map->entries[i].value;
            return true;
        }
    }
    return false;
}

void mrt_map_set(MrtMap *map, const char *key, Value val) {
    if (!map || !key) return;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            value_retain(val);
            value_release(map->entries[i].value);
            map->entries[i].value = val;
            return;
        }
    }

    if (map->count >= map->capacity) {
        map->capacity = map->capacity * 2;
        map->entries = (MrtMapEntry *)mrt_realloc(map->entries, sizeof(MrtMapEntry) * map->capacity);
    }

    value_retain(val);
    map->entries[map->count].key = mrt_strdup(key);
    map->entries[map->count].value = val;
    map->count++;
}

bool mrt_map_remove(MrtMap *map, const char *key, Value *out_removed) {
    if (!map || !key) return false;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->entries[i].key, key) == 0) {
            mrt_free(map->entries[i].key);
            if (out_removed) {
                *out_removed = map->entries[i].value;
            } else {
                value_release(map->entries[i].value);
            }
            for (size_t j = i; j + 1 < map->count; j++) {
                map->entries[j] = map->entries[j + 1];
            }
            map->count--;
            return true;
        }
    }
    return false;
}

bool mrt_map_has(MrtMap *map, const char *key) {
    return mrt_map_get(map, key, NULL);
}

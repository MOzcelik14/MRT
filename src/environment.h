#ifndef MRT_ENVIRONMENT_H
#define MRT_ENVIRONMENT_H

#include "common.h"
#include "value.h"

typedef struct EnvEntry {
    char *key;
    Value value;
    struct EnvEntry *next;
} EnvEntry;

typedef struct Environment {
    int ref_count;
    struct Environment *enclosing;
    EnvEntry **buckets;
    size_t capacity;
    size_t count;
} Environment;

Environment *env_new(Environment *enclosing);
void env_retain(Environment *env);
void env_release(Environment *env);

void env_define(Environment *env, const char *name, Value value);
bool env_assign(Environment *env, const char *name, Value value);
bool env_get(Environment *env, const char *name, Value *out_value);

#endif /* MRT_ENVIRONMENT_H */

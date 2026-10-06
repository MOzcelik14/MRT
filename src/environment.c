#include "environment.h"

#define ENV_INITIAL_CAPACITY 16

static uint32_t hash_string(const char *key) {
    uint32_t hash = 2166136261u;
    for (const char *p = key; *p; p++) {
        hash ^= (uint8_t)*p;
        hash *= 16777619;
    }
    return hash;
}

Environment *env_new(Environment *enclosing) {
    Environment *env = (Environment *)mrt_malloc(sizeof(Environment));
    env->ref_count = 1;
    env->enclosing = enclosing;
    if (enclosing) {
        env_retain(enclosing);
    }
    env->capacity = ENV_INITIAL_CAPACITY;
    env->count = 0;
    env->buckets = (EnvEntry **)mrt_calloc(env->capacity, sizeof(EnvEntry *));
    return env;
}

void env_retain(Environment *env) {
    if (env) {
        env->ref_count++;
    }
}

void env_release(Environment *env) {
    if (!env) return;
    env->ref_count--;
    if (env->ref_count <= 0) {
        for (size_t i = 0; i < env->capacity; i++) {
            EnvEntry *entry = env->buckets[i];
            while (entry) {
                EnvEntry *next = entry->next;
                mrt_free(entry->key);
                value_release(entry->value);
                mrt_free(entry);
                entry = next;
            }
        }
        mrt_free(env->buckets);

        if (env->enclosing) {
            env_release(env->enclosing);
            env->enclosing = NULL;
        }

        mrt_free(env);
    }
}

static void env_resize(Environment *env) {
    size_t new_cap = env->capacity * 2;
    EnvEntry **new_buckets = (EnvEntry **)mrt_calloc(new_cap, sizeof(EnvEntry *));

    for (size_t i = 0; i < env->capacity; i++) {
        EnvEntry *entry = env->buckets[i];
        while (entry) {
            EnvEntry *next = entry->next;
            uint32_t index = hash_string(entry->key) % new_cap;
            entry->next = new_buckets[index];
            new_buckets[index] = entry;
            entry = next;
        }
    }

    mrt_free(env->buckets);
    env->buckets = new_buckets;
    env->capacity = new_cap;
}

void env_define(Environment *env, const char *name, Value value) {
    if (!env || !name) return;

    if (env->count + 1 > env->capacity * 0.75) {
        env_resize(env);
    }

    uint32_t index = hash_string(name) % env->capacity;
    EnvEntry *entry = env->buckets[index];

    while (entry) {
        if (strcmp(entry->key, name) == 0) {
            /* Overwrite in current scope */
            value_release(entry->value);
            entry->value = value_copy(value);
            return;
        }
        entry = entry->next;
    }

    EnvEntry *new_entry = (EnvEntry *)mrt_malloc(sizeof(EnvEntry));
    new_entry->key = mrt_strdup(name);
    new_entry->value = value_copy(value);
    new_entry->next = env->buckets[index];
    env->buckets[index] = new_entry;
    env->count++;
}

bool env_assign(Environment *env, const char *name, Value value) {
    if (!env || !name) return false;

    uint32_t index = hash_string(name) % env->capacity;
    EnvEntry *entry = env->buckets[index];

    while (entry) {
        if (strcmp(entry->key, name) == 0) {
            value_release(entry->value);
            entry->value = value_copy(value);
            return true;
        }
        entry = entry->next;
    }

    if (env->enclosing) {
        return env_assign(env->enclosing, name, value);
    }

    return false;
}

bool env_get(Environment *env, const char *name, Value *out_value) {
    if (!env || !name) return false;

    uint32_t index = hash_string(name) % env->capacity;
    EnvEntry *entry = env->buckets[index];

    while (entry) {
        if (strcmp(entry->key, name) == 0) {
            if (out_value) {
                *out_value = value_copy(entry->value);
            }
            return true;
        }
        entry = entry->next;
    }

    if (env->enclosing) {
        return env_get(env->enclosing, name, out_value);
    }

    return false;
}

#ifndef MRT_COMMON_H
#define MRT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "version.h"

/* Memory management wrappers */
static inline void *mrt_malloc(size_t size) {
    if (size == 0) return NULL;
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Fatal error: out of memory (requested %zu bytes)\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static inline void *mrt_calloc(size_t count, size_t size) {
    if (count == 0 || size == 0) return NULL;
    void *ptr = calloc(count, size);
    if (!ptr) {
        fprintf(stderr, "Fatal error: out of memory (requested %zu elements of %zu bytes)\n", count, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static inline void *mrt_realloc(void *ptr, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    void *new_ptr = realloc(ptr, new_size);
    if (!new_ptr) {
        fprintf(stderr, "Fatal error: out of memory (realloc %zu bytes)\n", new_size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

static inline void mrt_free(void *ptr) {
    free(ptr);
}

static inline char *mrt_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)mrt_malloc(len + 1);
    memcpy(copy, s, len + 1);
    return copy;
}

static inline char *mrt_strndup(const char *s, size_t n) {
    if (!s) return NULL;
    char *copy = (char *)mrt_malloc(n + 1);
    memcpy(copy, s, n);
    copy[n] = '\0';
    return copy;
}

#endif /* MRT_COMMON_H */

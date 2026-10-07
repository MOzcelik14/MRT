#include "builtin.h"
#include "array.h"
#include "map.h"
#include "interpreter.h"
#include <time.h>

static Value builtin_typeof(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "typeOf() takes exactly 1 argument (%zu given)", arg_count);
        return value_none();
    }
    return value_string_from_cstr(value_type_name(args[0]));
}

static Value builtin_len(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "length() takes exactly 1 argument (%zu given)", arg_count);
        return value_none();
    }

    if (args[0].type == VAL_STRING) {
        return value_int((int64_t)args[0].as.string_val->length);
    } else if (args[0].type == VAL_ARRAY) {
        return value_int((int64_t)args[0].as.array_val->count);
    } else if (args[0].type == VAL_MAP) {
        return value_int((int64_t)args[0].as.map_val->count);
    }

    *had_error = true;
    mrt_report_error(interp->filename, interp->source, 0, 0,
                     ERR_TYPE, "length() expects string, array, or map argument, got %s",
                     value_type_name(args[0]));
    return value_none();
}

static Value builtin_str(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "toText() takes exactly 1 argument (%zu given)", arg_count);
        return value_none();
    }

    char *s = value_to_string(args[0]);
    Value val = value_string_from_cstr(s);
    mrt_free(s);
    return val;
}

static Value builtin_clock(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)arg_count;
    (void)args;
    (void)interp;
    (void)had_error;
    return value_float((double)clock() / CLOCKS_PER_SEC);
}

static char *read_line_from_stream(FILE *stream) {
    size_t capacity = 128;
    size_t len = 0;
    char *buffer = (char *)mrt_malloc(capacity);

    while (fgets(buffer + len, (int)(capacity - len), stream)) {
        len += strlen(buffer + len);
        if (len > 0 && buffer[len - 1] == '\n') {
            break;
        }
        if (capacity - len <= 1) {
            capacity *= 2;
            buffer = (char *)mrt_realloc(buffer, capacity);
        }
    }

    if (len == 0) {
        mrt_free(buffer);
        return NULL;
    }

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[--len] = '\0';
    }
    if (len > 0 && buffer[len - 1] == '\r') {
        buffer[--len] = '\0';
    }

    return buffer;
}

static Value builtin_read(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count > 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "read() takes at most 1 argument (%zu given)", arg_count);
        return value_none();
    }

    if (arg_count == 1) {
        char *prompt = value_to_string(args[0]);
        fputs(prompt, stdout);
        fflush(stdout);
        mrt_free(prompt);
    }

    char *line = read_line_from_stream(stdin);
    if (!line) {
        return value_none();
    }

    size_t len = strlen(line);
    return value_string_from_buffer(line, len);
}

static Value builtin_to_number(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "number() takes exactly 1 argument (%zu given)", arg_count);
        return value_none();
    }

    Value arg = args[0];
    if (arg.type == VAL_INT) {
        return arg;
    }
    if (arg.type == VAL_FLOAT) {
        return arg;
    }
    if (arg.type == VAL_BOOL) {
        return value_int(arg.as.bool_val ? 1 : 0);
    }
    if (arg.type != VAL_STRING) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "number() expects string, integer, float, or boolean, got %s",
                         value_type_name(arg));
        return value_none();
    }

    const char *str = arg.as.string_val->chars;
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "number() cannot convert empty string to number");
        return value_none();
    }

    bool has_dot_or_exp = false;
    for (const char *p = str; *p; p++) {
        if (*p == '.' || *p == 'e' || *p == 'E') {
            has_dot_or_exp = true;
            break;
        }
    }

    char *endptr = NULL;
    if (has_dot_or_exp) {
        double d = strtod(str, &endptr);
        while (isspace((unsigned char)*endptr)) endptr++;
        if (*endptr != '\0' || endptr == str) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "number() cannot convert '%s' to number", arg.as.string_val->chars);
            return value_none();
        }
        return value_float(d);
    } else {
        int64_t n = strtoll(str, &endptr, 10);
        while (isspace((unsigned char)*endptr)) endptr++;
        if (*endptr != '\0' || endptr == str) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "number() cannot convert '%s' to number", arg.as.string_val->chars);
            return value_none();
        }
        return value_int(n);
    }
}

static Value builtin_append(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 2) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "append() takes exactly 2 arguments (%zu given)", arg_count);
        return value_none();
    }

    if (args[0].type != VAL_ARRAY) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "append() expects array as first argument, got %s",
                         value_type_name(args[0]));
        return value_none();
    }

    mrt_array_push(args[0].as.array_val, args[1]);
    return value_copy(args[0]);
}

static Value builtin_remove(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 2) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "remove() takes exactly 2 arguments (%zu given)", arg_count);
        return value_none();
    }

    if (args[0].type == VAL_ARRAY) {
        if (args[1].type != VAL_INT) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "remove() from array expects integer index, got %s",
                             value_type_name(args[1]));
            return value_none();
        }
        int64_t idx = args[1].as.int_val;
        MrtArray *arr = args[0].as.array_val;
        if (idx < 0 || (size_t)idx >= arr->count) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_RUNTIME, "array index %ld out of bounds (length %zu)", (long)idx, arr->count);
            return value_none();
        }
        Value removed;
        mrt_array_remove_at(arr, (size_t)idx, &removed);
        return removed;
    } else if (args[0].type == VAL_MAP) {
        if (args[1].type != VAL_STRING) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "remove() from map expects string key, got %s",
                             value_type_name(args[1]));
            return value_none();
        }
        MrtMap *map = args[0].as.map_val;
        Value removed;
        if (!mrt_map_remove(map, args[1].as.string_val->chars, &removed)) {
            return value_none();
        }
        return removed;
    }

    *had_error = true;
    mrt_report_error(interp->filename, interp->source, 0, 0,
                     ERR_TYPE, "remove() expects array or map as first argument, got %s",
                     value_type_name(args[0]));
    return value_none();
}

static Value builtin_contains(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 2) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "contains() takes exactly 2 arguments (%zu given)", arg_count);
        return value_none();
    }

    if (args[0].type == VAL_ARRAY) {
        MrtArray *arr = args[0].as.array_val;
        for (size_t i = 0; i < arr->count; i++) {
            if (value_equal(arr->elements[i], args[1])) {
                return value_bool(true);
            }
        }
        return value_bool(false);
    } else if (args[0].type == VAL_MAP) {
        if (args[1].type != VAL_STRING) {
            return value_bool(false);
        }
        return value_bool(mrt_map_has(args[0].as.map_val, args[1].as.string_val->chars));
    } else if (args[0].type == VAL_STRING) {
        if (args[1].type != VAL_STRING) {
            return value_bool(false);
        }
        return value_bool(strstr(args[0].as.string_val->chars, args[1].as.string_val->chars) != NULL);
    }

    *had_error = true;
    mrt_report_error(interp->filename, interp->source, 0, 0,
                     ERR_TYPE, "contains() expects array, map, or string, got %s",
                     value_type_name(args[0]));
    return value_none();
}

static Value builtin_keys(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "keys() takes exactly 1 argument (%zu given)", arg_count);
        return value_none();
    }

    if (args[0].type != VAL_MAP) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "keys() expects map argument, got %s",
                         value_type_name(args[0]));
        return value_none();
    }

    MrtMap *map = args[0].as.map_val;
    MrtArray *arr = mrt_array_with_capacity(map->count);
    for (size_t i = 0; i < map->count; i++) {
        Value k = value_string_from_cstr(map->entries[i].key);
        mrt_array_push(arr, k);
        value_release(k);
    }
    return value_array(arr);
}

static Value builtin_values(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "values() takes exactly 1 argument (%zu given)", arg_count);
        return value_none();
    }

    if (args[0].type != VAL_MAP) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "values() expects map argument, got %s",
                         value_type_name(args[0]));
        return value_none();
    }

    MrtMap *map = args[0].as.map_val;
    MrtArray *arr = mrt_array_with_capacity(map->count);
    for (size_t i = 0; i < map->count; i++) {
        mrt_array_push(arr, map->entries[i].value);
    }
    return value_array(arr);
}

static Value builtin_range(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count < 1 || arg_count > 3) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "range() takes 1 to 3 arguments (%zu given)", arg_count);
        return value_none();
    }

    int64_t start = 0;
    int64_t end = 0;
    int64_t step = 1;

    if (arg_count == 1) {
        if (args[0].type != VAL_INT) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "range() argument must be integer");
            return value_none();
        }
        end = args[0].as.int_val;
    } else if (arg_count >= 2) {
        if (args[0].type != VAL_INT || args[1].type != VAL_INT) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "range() arguments must be integers");
            return value_none();
        }
        start = args[0].as.int_val;
        end = args[1].as.int_val;
        if (arg_count == 3) {
            if (args[2].type != VAL_INT || args[2].as.int_val == 0) {
                *had_error = true;
                mrt_report_error(interp->filename, interp->source, 0, 0,
                                 ERR_TYPE, "range() step must be a non-zero integer");
                return value_none();
            }
            step = args[2].as.int_val;
        }
    }

    size_t count = 0;
    if (step > 0 && start < end) {
        count = (size_t)((end - start + step - 1) / step);
    } else if (step < 0 && start > end) {
        count = (size_t)((start - end + (-step) - 1) / (-step));
    }

    MrtArray *arr = mrt_array_with_capacity(count);
    if (step > 0) {
        for (int64_t v = start; v < end; v += step) {
            Value iv = value_int(v);
            mrt_array_push(arr, iv);
        }
    } else {
        for (int64_t v = start; v > end; v += step) {
            Value iv = value_int(v);
            mrt_array_push(arr, iv);
        }
    }
    return value_array(arr);
}

static Value builtin_assert(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    if (arg_count < 1 || arg_count > 2) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "assert() takes 1 or 2 arguments (%zu given)", arg_count);
        return value_none();
    }

    if (!value_is_truthy(args[0])) {
        *had_error = true;
        char *msg = NULL;
        if (arg_count == 2) {
            msg = value_to_string(args[1]);
        } else {
            msg = mrt_strdup("assertion failed");
        }
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_RUNTIME, "%s", msg);
        mrt_free(msg);
        return value_none();
    }

    return value_bool(true);
}

void builtin_register_all(Environment *env) {
    env_define(env, "typeOf", value_native_fn(builtin_typeof));
    env_define(env, "length", value_native_fn(builtin_len));
    env_define(env, "toText", value_native_fn(builtin_str));
    env_define(env, "clock", value_native_fn(builtin_clock));
    env_define(env, "read", value_native_fn(builtin_read));
    env_define(env, "number", value_native_fn(builtin_to_number));
    env_define(env, "toNumber", value_native_fn(builtin_to_number));
    env_define(env, "append", value_native_fn(builtin_append));
    env_define(env, "remove", value_native_fn(builtin_remove));
    env_define(env, "contains", value_native_fn(builtin_contains));
    env_define(env, "keys", value_native_fn(builtin_keys));
    env_define(env, "values", value_native_fn(builtin_values));
    env_define(env, "range", value_native_fn(builtin_range));
    env_define(env, "assert", value_native_fn(builtin_assert));
}

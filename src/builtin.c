#include "builtin.h"
#include "interpreter.h"
#include <time.h>

static Value builtin_print(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    (void)had_error;
    for (size_t i = 0; i < arg_count; i++) {
        value_print(args[i]);
        if (i + 1 < arg_count) {
            printf(" ");
        }
    }
    printf("\n");
    fflush(stdout);
    return value_null();
}

static Value builtin_typeof(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "typeof() takes exactly 1 argument (%zu given)", arg_count);
        return value_null();
    }
    return value_string_from_cstr(value_type_name(args[0]));
}

static Value builtin_len(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "len() takes exactly 1 argument (%zu given)", arg_count);
        return value_null();
    }

    if (args[0].type != VAL_STRING) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "len() expects string argument, got %s", value_type_name(args[0]));
        return value_null();
    }

    return value_int((int64_t)args[0].as.string_val->length);
}

static Value builtin_str(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error) {
    (void)interp;
    if (arg_count != 1) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "str() takes exactly 1 argument (%zu given)", arg_count);
        return value_null();
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

void builtin_register_all(Environment *env) {
    env_define(env, "print", value_native_fn(builtin_print));
    env_define(env, "typeof", value_native_fn(builtin_typeof));
    env_define(env, "len", value_native_fn(builtin_len));
    env_define(env, "str", value_native_fn(builtin_str));
    env_define(env, "clock", value_native_fn(builtin_clock));
}

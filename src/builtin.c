#include "builtin.h"
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

    if (args[0].type != VAL_STRING) {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "length() expects string argument, got %s", value_type_name(args[0]));
        return value_none();
    }

    return value_int((int64_t)args[0].as.string_val->length);
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

void builtin_register_all(Environment *env) {
    env_define(env, "typeOf", value_native_fn(builtin_typeof));
    env_define(env, "length", value_native_fn(builtin_len));
    env_define(env, "toText", value_native_fn(builtin_str));
    env_define(env, "clock", value_native_fn(builtin_clock));
}

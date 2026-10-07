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
                         ERR_TYPE, "toNumber() takes exactly 1 argument (%zu given)", arg_count);
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
                         ERR_TYPE, "toNumber() expects string, integer, float, or boolean, got %s",
                         value_type_name(arg));
        return value_none();
    }

    const char *str = arg.as.string_val->chars;
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') {
        *had_error = true;
        mrt_report_error(interp->filename, interp->source, 0, 0,
                         ERR_TYPE, "toNumber() cannot convert empty string to number");
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
                             ERR_TYPE, "toNumber() cannot convert '%s' to number", arg.as.string_val->chars);
            return value_none();
        }
        return value_float(d);
    } else {
        int64_t n = strtoll(str, &endptr, 10);
        while (isspace((unsigned char)*endptr)) endptr++;
        if (*endptr != '\0' || endptr == str) {
            *had_error = true;
            mrt_report_error(interp->filename, interp->source, 0, 0,
                             ERR_TYPE, "toNumber() cannot convert '%s' to number", arg.as.string_val->chars);
            return value_none();
        }
        return value_int(n);
    }
}

void builtin_register_all(Environment *env) {
    env_define(env, "typeOf", value_native_fn(builtin_typeof));
    env_define(env, "length", value_native_fn(builtin_len));
    env_define(env, "toText", value_native_fn(builtin_str));
    env_define(env, "clock", value_native_fn(builtin_clock));
    env_define(env, "read", value_native_fn(builtin_read));
    env_define(env, "toNumber", value_native_fn(builtin_to_number));
}


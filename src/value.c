#include "value.h"
#include "environment.h"

Value value_none(void) {
    Value val;
    val.type = VAL_NONE;
    val.as.int_val = 0;
    return val;
}

Value value_null(void) {
    return value_none();
}

Value value_int(int64_t val) {
    Value v;
    v.type = VAL_INT;
    v.as.int_val = val;
    return v;
}

Value value_float(double val) {
    Value v;
    v.type = VAL_FLOAT;
    v.as.float_val = val;
    return v;
}

Value value_bool(bool val) {
    Value v;
    v.type = VAL_BOOL;
    v.as.bool_val = val;
    return v;
}

Value value_string(MrtString *str) {
    Value v;
    v.type = VAL_STRING;
    v.as.string_val = str;
    return v;
}

Value value_string_from_cstr(const char *chars) {
    size_t len = chars ? strlen(chars) : 0;
    MrtString *str = mrt_string_new(chars ? chars : "", len);
    return value_string(str);
}

Value value_string_from_buffer(char *chars, size_t length) {
    MrtString *str = mrt_string_take(chars, length);
    return value_string(str);
}

Value value_function(MrtFunction *fn) {
    Value v;
    v.type = VAL_FUNCTION;
    v.as.func_val = fn;
    return v;
}

Value value_native_fn(NativeFn fn) {
    Value v;
    v.type = VAL_NATIVE_FN;
    v.as.native_val = fn;
    return v;
}

MrtString *mrt_string_new(const char *chars, size_t length) {
    MrtString *str = (MrtString *)mrt_malloc(sizeof(MrtString));
    str->ref_count = 1;
    str->length = length;
    str->chars = (char *)mrt_malloc(length + 1);
    if (chars && length > 0) {
        memcpy(str->chars, chars, length);
    }
    str->chars[length] = '\0';
    return str;
}

MrtString *mrt_string_take(char *chars, size_t length) {
    MrtString *str = (MrtString *)mrt_malloc(sizeof(MrtString));
    str->ref_count = 1;
    str->length = length;
    str->chars = chars;
    return str;
}

void mrt_string_retain(MrtString *str) {
    if (str) {
        str->ref_count++;
    }
}

void mrt_string_release(MrtString *str) {
    if (!str) return;
    str->ref_count--;
    if (str->ref_count <= 0) {
        mrt_free(str->chars);
        mrt_free(str);
    }
}

MrtFunction *mrt_function_new(const char *name, char **params, size_t param_count,
                              struct ASTNode *body, struct Environment *closure) {
    MrtFunction *fn = (MrtFunction *)mrt_malloc(sizeof(MrtFunction));
    fn->ref_count = 1;
    fn->name = name ? mrt_strdup(name) : mrt_strdup("<anonymous>");
    fn->param_count = param_count;
    fn->params = (char **)mrt_malloc(param_count * sizeof(char *));
    for (size_t i = 0; i < param_count; i++) {
        fn->params[i] = mrt_strdup(params[i]);
    }
    fn->body = body;
    fn->closure = closure;
    return fn;
}

void mrt_function_retain(MrtFunction *fn) {
    if (fn) {
        fn->ref_count++;
    }
}

void mrt_function_release(MrtFunction *fn) {
    if (!fn) return;
    fn->ref_count--;
    if (fn->ref_count <= 0) {
        mrt_free(fn->name);
        for (size_t i = 0; i < fn->param_count; i++) {
            mrt_free(fn->params[i]);
        }
        mrt_free(fn->params);
        mrt_free(fn);
    }
}

void value_retain(Value val) {
    if (val.type == VAL_STRING) {
        mrt_string_retain(val.as.string_val);
    } else if (val.type == VAL_FUNCTION) {
        mrt_function_retain(val.as.func_val);
    }
}

void value_release(Value val) {
    if (val.type == VAL_STRING) {
        mrt_string_release(val.as.string_val);
    } else if (val.type == VAL_FUNCTION) {
        mrt_function_release(val.as.func_val);
    }
}

Value value_copy(Value val) {
    value_retain(val);
    return val;
}

bool value_is_truthy(Value val) {
    if (val.type == VAL_NULL) return false;
    if (val.type == VAL_BOOL) return val.as.bool_val;
    return true;
}

bool value_equal(Value a, Value b) {
    if (a.type == VAL_INT && b.type == VAL_FLOAT) {
        return (double)a.as.int_val == b.as.float_val;
    }
    if (a.type == VAL_FLOAT && b.type == VAL_INT) {
        return a.as.float_val == (double)b.as.int_val;
    }
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_NULL:     return true;
        case VAL_INT:      return a.as.int_val == b.as.int_val;
        case VAL_FLOAT:    return a.as.float_val == b.as.float_val;
        case VAL_BOOL:     return a.as.bool_val == b.as.bool_val;
        case VAL_STRING:   return strcmp(a.as.string_val->chars, b.as.string_val->chars) == 0;
        case VAL_FUNCTION: return a.as.func_val == b.as.func_val;
        case VAL_NATIVE_FN:return a.as.native_val == b.as.native_val;
        default:           return false;
    }
}

const char *value_type_name(Value val) {
    switch (val.type) {
        case VAL_NONE:      return "none";
        case VAL_INT:       return "integer";
        case VAL_FLOAT:     return "float";
        case VAL_BOOL:      return "boolean";
        case VAL_STRING:    return "string";
        case VAL_FUNCTION:
        case VAL_NATIVE_FN: return "function";
        default:            return "unknown";
    }
}

char *value_to_string(Value val) {
    char buf[128];
    switch (val.type) {
        case VAL_NONE:
            return mrt_strdup("none");
        case VAL_INT:
            snprintf(buf, sizeof(buf), "%ld", (long)val.as.int_val);
            return mrt_strdup(buf);
        case VAL_FLOAT:
            snprintf(buf, sizeof(buf), "%g", val.as.float_val);
            return mrt_strdup(buf);
        case VAL_BOOL:
            return mrt_strdup(val.as.bool_val ? "yes" : "no");
        case VAL_STRING:
            return mrt_strdup(val.as.string_val->chars);
        case VAL_FUNCTION:
            snprintf(buf, sizeof(buf), "<task %s>", val.as.func_val->name);
            return mrt_strdup(buf);
        case VAL_NATIVE_FN:
            return mrt_strdup("<native task>");
        default:
            return mrt_strdup("<unknown>");
    }
}

void value_print(Value val) {
    switch (val.type) {
        case VAL_NONE:
            printf("none");
            break;
        case VAL_INT:
            printf("%ld", (long)val.as.int_val);
            break;
        case VAL_FLOAT:
            printf("%g", val.as.float_val);
            break;
        case VAL_BOOL:
            printf("%s", val.as.bool_val ? "yes" : "no");
            break;
        case VAL_STRING:
            printf("%s", val.as.string_val->chars);
            break;
        case VAL_FUNCTION:
            printf("<task %s>", val.as.func_val->name);
            break;
        case VAL_NATIVE_FN:
            printf("<native task>");
            break;
    }
}

void value_print_repr(Value val) {
    if (val.type == VAL_STRING) {
        printf("\"%s\"", val.as.string_val->chars);
    } else {
        value_print(val);
    }
}

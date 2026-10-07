#include "value.h"
#include "array.h"
#include "map.h"
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

Value value_array(MrtArray *arr) {
    Value v;
    v.type = VAL_ARRAY;
    v.as.array_val = arr;
    return v;
}

Value value_map(MrtMap *map) {
    Value v;
    v.type = VAL_MAP;
    v.as.map_val = map;
    return v;
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
    } else if (val.type == VAL_ARRAY) {
        mrt_array_retain(val.as.array_val);
    } else if (val.type == VAL_MAP) {
        mrt_map_retain(val.as.map_val);
    } else if (val.type == VAL_FUNCTION) {
        mrt_function_retain(val.as.func_val);
    }
}

void value_release(Value val) {
    if (val.type == VAL_STRING) {
        mrt_string_release(val.as.string_val);
    } else if (val.type == VAL_ARRAY) {
        mrt_array_release(val.as.array_val);
    } else if (val.type == VAL_MAP) {
        mrt_map_release(val.as.map_val);
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
        case VAL_ARRAY: {
            MrtArray *arr_a = a.as.array_val;
            MrtArray *arr_b = b.as.array_val;
            if (arr_a == arr_b) return true;
            if (arr_a->count != arr_b->count) return false;
            for (size_t i = 0; i < arr_a->count; i++) {
                if (!value_equal(arr_a->elements[i], arr_b->elements[i])) {
                    return false;
                }
            }
            return true;
        }
        case VAL_MAP: {
            MrtMap *map_a = a.as.map_val;
            MrtMap *map_b = b.as.map_val;
            if (map_a == map_b) return true;
            if (map_a->count != map_b->count) return false;
            for (size_t i = 0; i < map_a->count; i++) {
                Value val_b;
                if (!mrt_map_get(map_b, map_a->entries[i].key, &val_b)) {
                    return false;
                }
                if (!value_equal(map_a->entries[i].value, val_b)) {
                    return false;
                }
            }
            return true;
        }
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
        case VAL_ARRAY:     return "array";
        case VAL_MAP:       return "map";
        case VAL_FUNCTION:
        case VAL_NATIVE_FN: return "function";
        default:            return "unknown";
    }
}

static void str_builder_append(char **buf, size_t *len, size_t *cap, const char *str) {
    size_t slen = strlen(str);
    if (*len + slen + 1 >= *cap) {
        *cap = (*cap + slen + 1) * 2;
        *buf = (char *)mrt_realloc(*buf, *cap);
    }
    memcpy(*buf + *len, str, slen);
    *len += slen;
    (*buf)[*len] = '\0';
}

static char *value_to_repr_string(Value val) {
    if (val.type == VAL_STRING) {
        size_t slen = strlen(val.as.string_val->chars);
        char *s = (char *)mrt_malloc(slen + 3);
        snprintf(s, slen + 3, "\"%s\"", val.as.string_val->chars);
        return s;
    }
    return value_to_string(val);
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
        case VAL_ARRAY: {
            size_t cap = 64;
            size_t len = 0;
            char *res = (char *)mrt_malloc(cap);
            res[0] = '\0';
            str_builder_append(&res, &len, &cap, "[");
            for (size_t i = 0; i < val.as.array_val->count; i++) {
                if (i > 0) str_builder_append(&res, &len, &cap, ", ");
                char *elem_str = value_to_repr_string(val.as.array_val->elements[i]);
                str_builder_append(&res, &len, &cap, elem_str);
                mrt_free(elem_str);
            }
            str_builder_append(&res, &len, &cap, "]");
            return res;
        }
        case VAL_MAP: {
            size_t cap = 64;
            size_t len = 0;
            char *res = (char *)mrt_malloc(cap);
            res[0] = '\0';
            str_builder_append(&res, &len, &cap, "{");
            for (size_t i = 0; i < val.as.map_val->count; i++) {
                if (i > 0) str_builder_append(&res, &len, &cap, ", ");
                str_builder_append(&res, &len, &cap, "\"");
                str_builder_append(&res, &len, &cap, val.as.map_val->entries[i].key);
                str_builder_append(&res, &len, &cap, "\": ");
                char *vstr = value_to_repr_string(val.as.map_val->entries[i].value);
                str_builder_append(&res, &len, &cap, vstr);
                mrt_free(vstr);
            }
            str_builder_append(&res, &len, &cap, "}");
            return res;
        }
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
    char *s = value_to_string(val);
    printf("%s", s);
    mrt_free(s);
}

void value_print_repr(Value val) {
    char *s = value_to_repr_string(val);
    printf("%s", s);
    mrt_free(s);
}

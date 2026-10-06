#ifndef MRT_VALUE_H
#define MRT_VALUE_H

#include "common.h"

struct ASTNode;
struct Environment;
struct Interpreter;

typedef enum {
    VAL_NULL,
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_NATIVE_FN
} ValueType;

typedef struct Value Value;

typedef Value (*NativeFn)(size_t arg_count, Value *args, struct Interpreter *interp, bool *had_error);

typedef struct MrtString {
    int ref_count;
    size_t length;
    char *chars;
} MrtString;

typedef struct MrtFunction {
    int ref_count;
    char *name;
    char **params;
    size_t param_count;
    struct ASTNode *body;
    struct Environment *closure;
} MrtFunction;

struct Value {
    ValueType type;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        MrtString *string_val;
        MrtFunction *func_val;
        NativeFn native_val;
    } as;
};

/* Value Constructors */
Value value_null(void);
Value value_int(int64_t val);
Value value_float(double val);
Value value_bool(bool val);
Value value_string(MrtString *str);
Value value_string_from_cstr(const char *chars);
Value value_string_from_buffer(char *chars, size_t length);
Value value_function(MrtFunction *fn);
Value value_native_fn(NativeFn fn);

/* String Management */
MrtString *mrt_string_new(const char *chars, size_t length);
MrtString *mrt_string_take(char *chars, size_t length);
void mrt_string_retain(MrtString *str);
void mrt_string_release(MrtString *str);

/* Function Management */
MrtFunction *mrt_function_new(const char *name, char **params, size_t param_count,
                              struct ASTNode *body, struct Environment *closure);
void mrt_function_retain(MrtFunction *fn);
void mrt_function_release(MrtFunction *fn);

/* Value Operations */
void value_retain(Value val);
void value_release(Value val);
Value value_copy(Value val);

bool value_is_truthy(Value val);
bool value_equal(Value a, Value b);
const char *value_type_name(Value val);
char *value_to_string(Value val);
void value_print(Value val);
void value_print_repr(Value val);

#endif /* MRT_VALUE_H */

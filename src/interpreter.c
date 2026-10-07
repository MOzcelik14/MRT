#include "interpreter.h"
#include "array.h"
#include "map.h"
#include "builtin.h"
#include "lexer.h"
#include "parser.h"
#include <math.h>

static void report_runtime_error(Interpreter *interp, const ASTNode *node,
                                 ErrorType type, const char *fmt, ...) {
    interp->had_runtime_error = true;
    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    int line = node ? node->line : 0;
    int col = node ? node->column : 0;
    mrt_report_error(interp->filename, interp->source, line, col, type, "%s", message);

    if (interp->frame_count > 0) {
        for (size_t i = interp->frame_count; i > 0; i--) {
            CallFrame *f = &interp->frames[i - 1];
            fprintf(stderr, "  at %s(%s:%d:%d)\n",
                    f->task_name ? f->task_name : "anonymous",
                    f->filename ? f->filename : "unknown",
                    f->line, f->col);
        }
        fprintf(stderr, "\n");
    }
}

void interpreter_init(Interpreter *interp, const char *source, const char *filename) {
    interp->globals = env_new(NULL);
    builtin_register_all(interp->globals);
    interp->environment = interp->globals;
    interp->source = source;
    interp->filename = filename;
    interp->had_runtime_error = false;
    interp->owns_globals = true;

    interp->frame_count = 0;
    interp->loaded_modules = NULL;
    interp->loaded_count = 0;
    interp->loaded_cap = 0;
    interp->loading_modules = NULL;
    interp->loading_count = 0;
    interp->loading_cap = 0;
    interp->module_asts = NULL;
    interp->module_sources = NULL;
    interp->module_ast_count = 0;
    interp->module_ast_cap = 0;
}

void interpreter_init_with_env(Interpreter *interp, Environment *globals,
                               const char *source, const char *filename) {
    interp->globals = globals;
    interp->environment = globals;
    interp->source = source;
    interp->filename = filename;
    interp->had_runtime_error = false;
    interp->owns_globals = false;

    interp->frame_count = 0;
    interp->loaded_modules = NULL;
    interp->loaded_count = 0;
    interp->loaded_cap = 0;
    interp->loading_modules = NULL;
    interp->loading_count = 0;
    interp->loading_cap = 0;
    interp->module_asts = NULL;
    interp->module_sources = NULL;
    interp->module_ast_count = 0;
    interp->module_ast_cap = 0;
}

void interpreter_free(Interpreter *interp) {
    if (interp->owns_globals && interp->globals) {
        env_release(interp->globals);
        interp->globals = NULL;
        interp->environment = NULL;
    }
    for (size_t i = 0; i < interp->loaded_count; i++) {
        mrt_free(interp->loaded_modules[i]);
    }
    mrt_free(interp->loaded_modules);
    interp->loaded_modules = NULL;
    interp->loaded_count = 0;
    interp->loaded_cap = 0;

    for (size_t i = 0; i < interp->loading_count; i++) {
        mrt_free(interp->loading_modules[i]);
    }
    mrt_free(interp->loading_modules);
    interp->loading_modules = NULL;
    interp->loading_count = 0;
    interp->loading_cap = 0;

    for (size_t i = 0; i < interp->module_ast_count; i++) {
        ast_free(interp->module_asts[i]);
        mrt_free(interp->module_sources[i]);
    }
    mrt_free(interp->module_asts);
    interp->module_asts = NULL;
    mrt_free(interp->module_sources);
    interp->module_sources = NULL;
    interp->module_ast_count = 0;
    interp->module_ast_cap = 0;
}

static EvalResult make_ok(Value val) {
    EvalResult res;
    res.status = INTERP_OK;
    res.value = val;
    return res;
}

static EvalResult make_return(Value val) {
    EvalResult res;
    res.status = INTERP_RETURN;
    res.value = val;
    return res;
}

static EvalResult make_break(void) {
    EvalResult res;
    res.status = INTERP_BREAK;
    res.value = value_none();
    return res;
}

static EvalResult make_continue(void) {
    EvalResult res;
    res.status = INTERP_CONTINUE;
    res.value = value_none();
    return res;
}

static EvalResult make_error(void) {
    EvalResult res;
    res.status = INTERP_ERROR;
    res.value = value_none();
    return res;
}

static EvalResult eval_binary(Interpreter *interp, const ASTNode *node) {
    TokenType op = node->as.binary.op;

    /* Short-circuit logical operators */
    if (op == TOKEN_AND) {
        EvalResult left_res = interpreter_eval_node(interp, node->as.binary.left);
        if (left_res.status != INTERP_OK) return left_res;
        if (!value_is_truthy(left_res.value)) {
            return left_res;
        }
        value_release(left_res.value);
        return interpreter_eval_node(interp, node->as.binary.right);
    }

    if (op == TOKEN_OR) {
        EvalResult left_res = interpreter_eval_node(interp, node->as.binary.left);
        if (left_res.status != INTERP_OK) return left_res;
        if (value_is_truthy(left_res.value)) {
            return left_res;
        }
        value_release(left_res.value);
        return interpreter_eval_node(interp, node->as.binary.right);
    }

    EvalResult left_res = interpreter_eval_node(interp, node->as.binary.left);
    if (left_res.status != INTERP_OK) return left_res;

    EvalResult right_res = interpreter_eval_node(interp, node->as.binary.right);
    if (right_res.status != INTERP_OK) {
        value_release(left_res.value);
        return right_res;
    }

    Value l = left_res.value;
    Value r = right_res.value;

    switch (op) {
        case TOKEN_PLUS:
            if (l.type == VAL_INT && r.type == VAL_INT) {
                value_release(l);
                value_release(r);
                return make_ok(value_int(l.as.int_val + r.as.int_val));
            }
            if (l.type == VAL_INT && r.type == VAL_FLOAT) {
                value_release(l);
                value_release(r);
                return make_ok(value_float((double)l.as.int_val + r.as.float_val));
            }
            if (l.type == VAL_FLOAT && r.type == VAL_INT) {
                value_release(l);
                value_release(r);
                return make_ok(value_float(l.as.float_val + (double)r.as.int_val));
            }
            if (l.type == VAL_FLOAT && r.type == VAL_FLOAT) {
                value_release(l);
                value_release(r);
                return make_ok(value_float(l.as.float_val + r.as.float_val));
            }
            if (l.type == VAL_ARRAY && r.type == VAL_ARRAY) {
                MrtArray *arr1 = l.as.array_val;
                MrtArray *arr2 = r.as.array_val;
                MrtArray *res = mrt_array_with_capacity(arr1->count + arr2->count);
                for (size_t i = 0; i < arr1->count; i++) mrt_array_push(res, arr1->elements[i]);
                for (size_t i = 0; i < arr2->count; i++) mrt_array_push(res, arr2->elements[i]);
                value_release(l);
                value_release(r);
                return make_ok(value_array(res));
            }
            if (l.type == VAL_STRING || r.type == VAL_STRING) {
                char *s_l = (l.type == VAL_STRING) ? NULL : value_to_string(l);
                char *s_r = (r.type == VAL_STRING) ? NULL : value_to_string(r);
                const char *str1 = s_l ? s_l : l.as.string_val->chars;
                const char *str2 = s_r ? s_r : r.as.string_val->chars;
                size_t len1 = strlen(str1);
                size_t len2 = strlen(str2);
                char *concat = (char *)mrt_malloc(len1 + len2 + 1);
                memcpy(concat, str1, len1);
                memcpy(concat + len1, str2, len2 + 1);
                if (s_l) mrt_free(s_l);
                if (s_r) mrt_free(s_r);
                value_release(l);
                value_release(r);
                return make_ok(value_string_from_buffer(concat, len1 + len2));
            }
            report_runtime_error(interp, node, ERR_TYPE, "cannot add %s and %s",
                                 value_type_name(l), value_type_name(r));
            break;

        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT: {
            bool l_is_num = (l.type == VAL_INT || l.type == VAL_FLOAT);
            bool r_is_num = (r.type == VAL_INT || r.type == VAL_FLOAT);
            if (!l_is_num || !r_is_num) {
                report_runtime_error(interp, node, ERR_TYPE,
                                     "operands must be numbers for arithmetic operator");
                break;
            }

            double num_l = (l.type == VAL_INT) ? (double)l.as.int_val : l.as.float_val;
            double num_r = (r.type == VAL_INT) ? (double)r.as.int_val : r.as.float_val;

            if (op == TOKEN_SLASH) {
                if (num_r == 0.0) {
                    report_runtime_error(interp, node, ERR_RUNTIME, "division by zero");
                    break;
                }
                value_release(l);
                value_release(r);
                if (l.type == VAL_INT && r.type == VAL_INT && (l.as.int_val % r.as.int_val == 0)) {
                    return make_ok(value_int(l.as.int_val / r.as.int_val));
                }
                return make_ok(value_float(num_l / num_r));
            }

            if (op == TOKEN_PERCENT) {
                if (l.type != VAL_INT || r.type != VAL_INT) {
                    report_runtime_error(interp, node, ERR_TYPE,
                                         "modulo operands must be integers");
                    break;
                }
                if (r.as.int_val == 0) {
                    report_runtime_error(interp, node, ERR_RUNTIME, "division by zero");
                    break;
                }
                int64_t res = l.as.int_val % r.as.int_val;
                value_release(l);
                value_release(r);
                return make_ok(value_int(res));
            }

            if (op == TOKEN_MINUS) {
                value_release(l);
                value_release(r);
                if (l.type == VAL_INT && r.type == VAL_INT) {
                    return make_ok(value_int(l.as.int_val - r.as.int_val));
                }
                return make_ok(value_float(num_l - num_r));
            }

            if (op == TOKEN_STAR) {
                value_release(l);
                value_release(r);
                if (l.type == VAL_INT && r.type == VAL_INT) {
                    return make_ok(value_int(l.as.int_val * r.as.int_val));
                }
                return make_ok(value_float(num_l * num_r));
            }
            break;
        }

        case TOKEN_EQUAL_EQUAL: {
            bool eq = value_equal(l, r);
            value_release(l);
            value_release(r);
            return make_ok(value_bool(eq));
        }

        case TOKEN_BANG_EQUAL: {
            bool eq = value_equal(l, r);
            value_release(l);
            value_release(r);
            return make_ok(value_bool(!eq));
        }

        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL: {
            if ((l.type == VAL_INT || l.type == VAL_FLOAT) &&
                (r.type == VAL_INT || r.type == VAL_FLOAT)) {
                double nl = (l.type == VAL_INT) ? (double)l.as.int_val : l.as.float_val;
                double nr = (r.type == VAL_INT) ? (double)r.as.int_val : r.as.float_val;
                value_release(l);
                value_release(r);
                switch (op) {
                    case TOKEN_LESS:          return make_ok(value_bool(nl < nr));
                    case TOKEN_LESS_EQUAL:    return make_ok(value_bool(nl <= nr));
                    case TOKEN_GREATER:       return make_ok(value_bool(nl > nr));
                    case TOKEN_GREATER_EQUAL: return make_ok(value_bool(nl >= nr));
                    default: break;
                }
            } else if (l.type == VAL_STRING && r.type == VAL_STRING) {
                int cmp = strcmp(l.as.string_val->chars, r.as.string_val->chars);
                value_release(l);
                value_release(r);
                switch (op) {
                    case TOKEN_LESS:          return make_ok(value_bool(cmp < 0));
                    case TOKEN_LESS_EQUAL:    return make_ok(value_bool(cmp <= 0));
                    case TOKEN_GREATER:       return make_ok(value_bool(cmp > 0));
                    case TOKEN_GREATER_EQUAL: return make_ok(value_bool(cmp >= 0));
                    default: break;
                }
            }
            report_runtime_error(interp, node, ERR_TYPE,
                                 "comparison not supported between %s and %s",
                                 value_type_name(l), value_type_name(r));
            break;
        }

        default:
            report_runtime_error(interp, node, ERR_RUNTIME, "unsupported binary operator");
            break;
    }

    value_release(l);
    value_release(r);
    return make_error();
}

static EvalResult eval_unary(Interpreter *interp, const ASTNode *node) {
    TokenType op = node->as.unary.op;
    EvalResult operand_res = interpreter_eval_node(interp, node->as.unary.operand);
    if (operand_res.status != INTERP_OK) return operand_res;

    Value val = operand_res.value;

    if (op == TOKEN_MINUS) {
        if (val.type == VAL_INT) {
            val.as.int_val = -val.as.int_val;
            return make_ok(val);
        }
        if (val.type == VAL_FLOAT) {
            val.as.float_val = -val.as.float_val;
            return make_ok(val);
        }
        report_runtime_error(interp, node, ERR_TYPE,
                             "unary minus requires numeric operand, got %s",
                             value_type_name(val));
        value_release(val);
        return make_error();
    }

    if (op == TOKEN_NOT) {
        bool b = !value_is_truthy(val);
        value_release(val);
        return make_ok(value_bool(b));
    }

    value_release(val);
    report_runtime_error(interp, node, ERR_RUNTIME, "unsupported unary operator");
    return make_error();
}

static EvalResult eval_call(Interpreter *interp, const ASTNode *node) {
    EvalResult callee_res = interpreter_eval_node(interp, node->as.func_call.callee);
    if (callee_res.status != INTERP_OK) return callee_res;

    Value callee = callee_res.value;
    size_t arg_count = node->as.func_call.arg_count;
    Value *eval_args = NULL;

    if (arg_count > 0) {
        eval_args = (Value *)mrt_malloc(arg_count * sizeof(Value));
        for (size_t i = 0; i < arg_count; i++) {
            EvalResult arg_res = interpreter_eval_node(interp, node->as.func_call.args[i]);
            if (arg_res.status != INTERP_OK) {
                for (size_t j = 0; j < i; j++) {
                    value_release(eval_args[j]);
                }
                mrt_free(eval_args);
                value_release(callee);
                return arg_res;
            }
            eval_args[i] = arg_res.value;
        }
    }

    if (callee.type == VAL_FUNCTION) {
        MrtFunction *fn = callee.as.func_val;
        if (arg_count != fn->param_count) {
            report_runtime_error(interp, node, ERR_TYPE,
                                 "function '%s' expects %zu argument(s) but got %zu",
                                 fn->name, fn->param_count, arg_count);
            for (size_t i = 0; i < arg_count; i++) value_release(eval_args[i]);
            mrt_free(eval_args);
            value_release(callee);
            return make_error();
        }

        Environment *call_env = env_new(fn->closure);
        for (size_t i = 0; i < arg_count; i++) {
            env_define(call_env, fn->params[i], eval_args[i]);
            value_release(eval_args[i]);
        }
        mrt_free(eval_args);

        Environment *prev_env = interp->environment;
        interp->environment = call_env;

        if (interp->frame_count < MAX_CALL_FRAMES) {
            interp->frames[interp->frame_count++] = (CallFrame){
                .task_name = fn->name,
                .filename = interp->filename,
                .line = node->line,
                .col = node->column
            };
        }

        EvalResult body_res = interpreter_eval_node(interp, fn->body);

        if (interp->frame_count > 0) {
            interp->frame_count--;
        }

        interp->environment = prev_env;
        env_release(call_env);
        value_release(callee);

        if (body_res.status == INTERP_RETURN) {
            body_res.status = INTERP_OK;
            return body_res;
        }
        if (body_res.status == INTERP_OK) {
            value_release(body_res.value);
            return make_ok(value_null());
        }
        return body_res;
    }

    if (callee.type == VAL_NATIVE_FN) {
        bool had_error = false;
        Value result = callee.as.native_val(arg_count, eval_args, interp, &had_error);
        for (size_t i = 0; i < arg_count; i++) {
            value_release(eval_args[i]);
        }
        mrt_free(eval_args);
        value_release(callee);

        if (had_error) {
            value_release(result);
            return make_error();
        }
        return make_ok(result);
    }

    report_runtime_error(interp, node, ERR_TYPE, "can only call functions, got %s",
                         value_type_name(callee));
    for (size_t i = 0; i < arg_count; i++) value_release(eval_args[i]);
    mrt_free(eval_args);
    value_release(callee);
    return make_error();
}

static char *read_file_content(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    char *buffer = (char *)mrt_malloc((size_t)size + 1);
    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    return buffer;
}

static char *resolve_module_path(const char *current_file, const char *import_path) {
    if (!import_path) return NULL;
    if (import_path[0] == '/' || !current_file) {
        return mrt_strdup(import_path);
    }
    const char *last_slash = strrchr(current_file, '/');
    if (!last_slash) {
        return mrt_strdup(import_path);
    }
    size_t dir_len = (size_t)(last_slash - current_file);
    size_t imp_len = strlen(import_path);
    char *resolved = (char *)mrt_malloc(dir_len + 1 + imp_len + 1);
    memcpy(resolved, current_file, dir_len);
    resolved[dir_len] = '/';
    memcpy(resolved + dir_len + 1, import_path, imp_len);
    resolved[dir_len + 1 + imp_len] = '\0';
    return resolved;
}

EvalResult interpreter_eval_node(Interpreter *interp, ASTNode *node) {
    if (!node) return make_ok(value_null());

    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK: {
            bool is_block = (node->type == AST_BLOCK);
            Environment *prev_env = interp->environment;
            Environment *local_env = NULL;

            if (is_block) {
                local_env = env_new(prev_env);
                interp->environment = local_env;
            }

            EvalResult last_res = make_ok(value_null());

            for (size_t i = 0; i < node->as.block.count; i++) {
                value_release(last_res.value);
                last_res = interpreter_eval_node(interp, node->as.block.statements[i]);
                if (last_res.status != INTERP_OK) {
                    break;
                }
            }

            if (is_block) {
                interp->environment = prev_env;
                env_release(local_env);
            } else if (last_res.status == INTERP_RETURN) {
                last_res.status = INTERP_OK;
            }

            return last_res;
        }

        case AST_VAR_DECL: {
            Value init_val = value_null();
            if (node->as.var_decl.init) {
                EvalResult res = interpreter_eval_node(interp, node->as.var_decl.init);
                if (res.status != INTERP_OK) return res;
                init_val = res.value;
            }
            env_define(interp->environment, node->as.var_decl.name, init_val);
            value_release(init_val);
            return make_ok(value_null());
        }

        case AST_ASSIGN: {
            EvalResult res = interpreter_eval_node(interp, node->as.assign.value);
            if (res.status != INTERP_OK) return res;
            if (!env_assign(interp->environment, node->as.assign.name, res.value)) {
                report_runtime_error(interp, node, ERR_NAME,
                                     "undefined variable '%s'", node->as.assign.name);
                value_release(res.value);
                return make_error();
            }
            return res;
        }

        case AST_LITERAL: {
            switch (node->as.literal.lit_type) {
                case LITERAL_INT:
                    return make_ok(value_int(node->as.literal.as.int_val));
                case LITERAL_FLOAT:
                    return make_ok(value_float(node->as.literal.as.float_val));
                case LITERAL_STRING:
                    return make_ok(value_string_from_cstr(node->as.literal.as.string_val));
                case LITERAL_BOOL:
                    return make_ok(value_bool(node->as.literal.as.bool_val));
                case LITERAL_NULL:
                    return make_ok(value_null());
            }
            return make_ok(value_null());
        }

        case AST_ARRAY_LITERAL: {
            MrtArray *arr = mrt_array_with_capacity(node->as.array_literal.count);
            for (size_t i = 0; i < node->as.array_literal.count; i++) {
                EvalResult elem_res = interpreter_eval_node(interp, node->as.array_literal.elements[i]);
                if (elem_res.status != INTERP_OK) {
                    mrt_array_release(arr);
                    return elem_res;
                }
                mrt_array_push(arr, elem_res.value);
                value_release(elem_res.value);
            }
            return make_ok(value_array(arr));
        }

        case AST_MAP_LITERAL: {
            MrtMap *map = mrt_map_new();
            for (size_t i = 0; i < node->as.map_literal.count; i++) {
                EvalResult val_res = interpreter_eval_node(interp, node->as.map_literal.values[i]);
                if (val_res.status != INTERP_OK) {
                    mrt_map_release(map);
                    return val_res;
                }
                mrt_map_set(map, node->as.map_literal.keys[i], val_res.value);
                value_release(val_res.value);
            }
            return make_ok(value_map(map));
        }

        case AST_INDEX_GET: {
            EvalResult target_res = interpreter_eval_node(interp, node->as.index_get.target);
            if (target_res.status != INTERP_OK) return target_res;

            EvalResult index_res = interpreter_eval_node(interp, node->as.index_get.index);
            if (index_res.status != INTERP_OK) {
                value_release(target_res.value);
                return index_res;
            }

            Value target = target_res.value;
            Value index = index_res.value;

            if (target.type == VAL_ARRAY) {
                if (index.type != VAL_INT) {
                    report_runtime_error(interp, node, ERR_TYPE,
                                         "array index must be integer, got %s", value_type_name(index));
                    value_release(target);
                    value_release(index);
                    return make_error();
                }
                int64_t idx = index.as.int_val;
                MrtArray *arr = target.as.array_val;
                if (idx < 0 || (size_t)idx >= arr->count) {
                    report_runtime_error(interp, node, ERR_RUNTIME,
                                         "array index %ld out of bounds (length %zu)", (long)idx, arr->count);
                    value_release(target);
                    value_release(index);
                    return make_error();
                }
                Value item = mrt_array_get(arr, (size_t)idx);
                value_retain(item);
                value_release(target);
                value_release(index);
                return make_ok(item);
            } else if (target.type == VAL_MAP) {
                if (index.type != VAL_STRING) {
                    report_runtime_error(interp, node, ERR_TYPE,
                                         "map key must be string, got %s", value_type_name(index));
                    value_release(target);
                    value_release(index);
                    return make_error();
                }
                MrtMap *map = target.as.map_val;
                Value val;
                if (!mrt_map_get(map, index.as.string_val->chars, &val)) {
                    report_runtime_error(interp, node, ERR_RUNTIME,
                                         "key '%s' not found in map", index.as.string_val->chars);
                    value_release(target);
                    value_release(index);
                    return make_error();
                }
                value_retain(val);
                value_release(target);
                value_release(index);
                return make_ok(val);
            } else if (target.type == VAL_STRING) {
                if (index.type != VAL_INT) {
                    report_runtime_error(interp, node, ERR_TYPE,
                                         "string index must be integer, got %s", value_type_name(index));
                    value_release(target);
                    value_release(index);
                    return make_error();
                }
                int64_t idx = index.as.int_val;
                MrtString *str = target.as.string_val;
                if (idx < 0 || (size_t)idx >= str->length) {
                    report_runtime_error(interp, node, ERR_RUNTIME,
                                         "string index %ld out of bounds (length %zu)", (long)idx, str->length);
                    value_release(target);
                    value_release(index);
                    return make_error();
                }
                char ch[2] = { str->chars[idx], '\0' };
                Value res = value_string_from_cstr(ch);
                value_release(target);
                value_release(index);
                return make_ok(res);
            }

            report_runtime_error(interp, node, ERR_TYPE,
                                 "type '%s' does not support indexing", value_type_name(target));
            value_release(target);
            value_release(index);
            return make_error();
        }

        case AST_INDEX_SET: {
            EvalResult target_res = interpreter_eval_node(interp, node->as.index_set.target);
            if (target_res.status != INTERP_OK) return target_res;

            EvalResult index_res = interpreter_eval_node(interp, node->as.index_set.index);
            if (index_res.status != INTERP_OK) {
                value_release(target_res.value);
                return index_res;
            }

            EvalResult val_res = interpreter_eval_node(interp, node->as.index_set.value);
            if (val_res.status != INTERP_OK) {
                value_release(target_res.value);
                value_release(index_res.value);
                return val_res;
            }

            Value target = target_res.value;
            Value index = index_res.value;
            Value val = val_res.value;

            if (target.type == VAL_ARRAY) {
                if (index.type != VAL_INT) {
                    report_runtime_error(interp, node, ERR_TYPE,
                                         "array index must be integer, got %s", value_type_name(index));
                    value_release(target);
                    value_release(index);
                    value_release(val);
                    return make_error();
                }
                int64_t idx = index.as.int_val;
                MrtArray *arr = target.as.array_val;
                if (idx < 0 || (size_t)idx >= arr->count) {
                    report_runtime_error(interp, node, ERR_RUNTIME,
                                         "array index %ld out of bounds (length %zu)", (long)idx, arr->count);
                    value_release(target);
                    value_release(index);
                    value_release(val);
                    return make_error();
                }
                mrt_array_set(arr, (size_t)idx, val);
                value_release(target);
                value_release(index);
                return make_ok(val);
            } else if (target.type == VAL_MAP) {
                if (index.type != VAL_STRING) {
                    report_runtime_error(interp, node, ERR_TYPE,
                                         "map key must be string, got %s", value_type_name(index));
                    value_release(target);
                    value_release(index);
                    value_release(val);
                    return make_error();
                }
                MrtMap *map = target.as.map_val;
                mrt_map_set(map, index.as.string_val->chars, val);
                value_release(target);
                value_release(index);
                return make_ok(val);
            }

            report_runtime_error(interp, node, ERR_TYPE,
                                 "type '%s' does not support index assignment", value_type_name(target));
            value_release(target);
            value_release(index);
            value_release(val);
            return make_error();
        }

        case AST_IDENTIFIER: {
            Value val;
            if (!env_get(interp->environment, node->as.identifier.name, &val)) {
                report_runtime_error(interp, node, ERR_NAME,
                                     "undefined variable '%s'", node->as.identifier.name);
                return make_error();
            }
            return make_ok(val);
        }

        case AST_BINARY:
            return eval_binary(interp, node);

        case AST_UNARY:
            return eval_unary(interp, node);

        case AST_FUNCTION_DECL: {
            MrtFunction *fn = mrt_function_new(node->as.func_decl.name,
                                               node->as.func_decl.params,
                                               node->as.func_decl.param_count,
                                               node->as.func_decl.body,
                                               interp->environment);
            env_define(interp->environment, node->as.func_decl.name, value_function(fn));
            mrt_function_release(fn);
            return make_ok(value_null());
        }

        case AST_FUNCTION_CALL:
            return eval_call(interp, node);

        case AST_IF: {
            EvalResult cond_res = interpreter_eval_node(interp, node->as.if_stmt.condition);
            if (cond_res.status != INTERP_OK) return cond_res;

            bool truthy = value_is_truthy(cond_res.value);
            value_release(cond_res.value);

            if (truthy) {
                return interpreter_eval_node(interp, node->as.if_stmt.then_branch);
            } else if (node->as.if_stmt.else_branch) {
                return interpreter_eval_node(interp, node->as.if_stmt.else_branch);
            }
            return make_ok(value_null());
        }

        case AST_WHILE: {
            for (;;) {
                EvalResult cond_res = interpreter_eval_node(interp, node->as.while_stmt.condition);
                if (cond_res.status != INTERP_OK) return cond_res;

                bool truthy = value_is_truthy(cond_res.value);
                value_release(cond_res.value);

                if (!truthy) break;

                EvalResult body_res = interpreter_eval_node(interp, node->as.while_stmt.body);
                if (body_res.status == INTERP_BREAK) {
                    value_release(body_res.value);
                    break;
                }
                if (body_res.status == INTERP_CONTINUE) {
                    value_release(body_res.value);
                    continue;
                }
                if (body_res.status != INTERP_OK) {
                    return body_res;
                }
                value_release(body_res.value);
            }
            return make_ok(value_none());
        }

        case AST_EACH: {
            EvalResult coll_res = interpreter_eval_node(interp, node->as.each_stmt.collection);
            if (coll_res.status != INTERP_OK) return coll_res;

            Value coll = coll_res.value;
            const char *var_name = node->as.each_stmt.var_name;

            if (coll.type == VAL_ARRAY) {
                MrtArray *arr = coll.as.array_val;
                mrt_array_retain(arr);
                value_release(coll);

                for (size_t i = 0; i < arr->count; i++) {
                    env_define(interp->environment, var_name, arr->elements[i]);
                    EvalResult body_res = interpreter_eval_node(interp, node->as.each_stmt.body);
                    if (body_res.status == INTERP_BREAK) {
                        value_release(body_res.value);
                        break;
                    }
                    if (body_res.status == INTERP_CONTINUE) {
                        value_release(body_res.value);
                        continue;
                    }
                    if (body_res.status != INTERP_OK) {
                        mrt_array_release(arr);
                        return body_res;
                    }
                    value_release(body_res.value);
                }
                mrt_array_release(arr);
                return make_ok(value_none());
            } else if (coll.type == VAL_MAP) {
                MrtMap *map = coll.as.map_val;
                mrt_map_retain(map);
                value_release(coll);

                for (size_t i = 0; i < map->count; i++) {
                    Value key_val = value_string_from_cstr(map->entries[i].key);
                    env_define(interp->environment, var_name, key_val);
                    value_release(key_val);

                    EvalResult body_res = interpreter_eval_node(interp, node->as.each_stmt.body);
                    if (body_res.status == INTERP_BREAK) {
                        value_release(body_res.value);
                        break;
                    }
                    if (body_res.status == INTERP_CONTINUE) {
                        value_release(body_res.value);
                        continue;
                    }
                    if (body_res.status != INTERP_OK) {
                        mrt_map_release(map);
                        return body_res;
                    }
                    value_release(body_res.value);
                }
                mrt_map_release(map);
                return make_ok(value_none());
            } else if (coll.type == VAL_STRING) {
                MrtString *str = coll.as.string_val;
                mrt_string_retain(str);
                value_release(coll);

                for (size_t i = 0; i < str->length; i++) {
                    char ch[2] = { str->chars[i], '\0' };
                    Value ch_val = value_string_from_cstr(ch);
                    env_define(interp->environment, var_name, ch_val);
                    value_release(ch_val);

                    EvalResult body_res = interpreter_eval_node(interp, node->as.each_stmt.body);
                    if (body_res.status == INTERP_BREAK) {
                        value_release(body_res.value);
                        break;
                    }
                    if (body_res.status == INTERP_CONTINUE) {
                        value_release(body_res.value);
                        continue;
                    }
                    if (body_res.status != INTERP_OK) {
                        mrt_string_release(str);
                        return body_res;
                    }
                    value_release(body_res.value);
                }
                mrt_string_release(str);
                return make_ok(value_none());
            }

            report_runtime_error(interp, node, ERR_TYPE,
                                 "cannot iterate over type '%s' with each", value_type_name(coll));
            value_release(coll);
            return make_error();
        }

        case AST_RETURN: {
            Value ret_val = value_none();
            if (node->as.return_stmt.value) {
                EvalResult res = interpreter_eval_node(interp, node->as.return_stmt.value);
                if (res.status != INTERP_OK) return res;
                ret_val = res.value;
            }
            return make_return(ret_val);
        }

        case AST_SAY: {
            EvalResult res = interpreter_eval_node(interp, node->as.say_stmt.value);
            if (res.status != INTERP_OK) return res;
            value_print(res.value);
            printf("\n");
            fflush(stdout);
            value_release(res.value);
            return make_ok(value_none());
        }

        case AST_BREAK:
            return make_break();

        case AST_CONTINUE:
            return make_continue();

        case AST_USE: {
            char *resolved_path = resolve_module_path(interp->filename, node->as.use_stmt.path);
            if (!resolved_path) {
                report_runtime_error(interp, node, ERR_RUNTIME, "invalid module path");
                return make_error();
            }

            /* Check circular imports */
            for (size_t i = 0; i < interp->loading_count; i++) {
                if (strcmp(interp->loading_modules[i], resolved_path) == 0) {
                    report_runtime_error(interp, node, ERR_RUNTIME,
                                         "circular import detected: '%s'", node->as.use_stmt.path);
                    mrt_free(resolved_path);
                    return make_error();
                }
            }

            /* Check if already loaded */
            for (size_t i = 0; i < interp->loaded_count; i++) {
                if (strcmp(interp->loaded_modules[i], resolved_path) == 0) {
                    mrt_free(resolved_path);
                    return make_ok(value_none());
                }
            }

            /* Push to loading stack */
            if (interp->loading_count + 1 > interp->loading_cap) {
                interp->loading_cap = interp->loading_cap < 4 ? 4 : interp->loading_cap * 2;
                interp->loading_modules = (char **)mrt_realloc(
                    interp->loading_modules, sizeof(char *) * interp->loading_cap);
            }
            interp->loading_modules[interp->loading_count++] = mrt_strdup(resolved_path);

            char *mod_source = read_file_content(resolved_path);
            if (!mod_source) {
                report_runtime_error(interp, node, ERR_RUNTIME,
                                     "cannot open module file '%s'", resolved_path);
                mrt_free(interp->loading_modules[--interp->loading_count]);
                mrt_free(resolved_path);
                return make_error();
            }

            Lexer lexer;
            lexer_init(&lexer, mod_source, resolved_path);
            TokenArray tokens = lexer_tokenize_all(&lexer);
            if (lexer.had_error) {
                report_runtime_error(interp, node, ERR_SYNTAX,
                                     "syntax error in module '%s'", resolved_path);
                token_array_free(&tokens);
                mrt_free(mod_source);
                mrt_free(interp->loading_modules[--interp->loading_count]);
                mrt_free(resolved_path);
                return make_error();
            }

            Parser parser;
            parser_init(&parser, tokens, mod_source, resolved_path);
            ASTNode *mod_ast = parser_parse(&parser);
            if (!mod_ast || parser.had_error) {
                report_runtime_error(interp, node, ERR_SYNTAX,
                                     "syntax error in module '%s'", resolved_path);
                parser_free(&parser);
                mrt_free(mod_source);
                mrt_free(interp->loading_modules[--interp->loading_count]);
                mrt_free(resolved_path);
                return make_error();
            }

            const char *saved_fn = interp->filename;
            const char *saved_src = interp->source;
            Environment *saved_env = interp->environment;

            interp->filename = resolved_path;
            interp->source = mod_source;
            interp->environment = interp->globals;

            EvalResult mod_res = interpreter_eval_node(interp, mod_ast);

            interp->filename = saved_fn;
            interp->source = saved_src;
            interp->environment = saved_env;

            if (interp->module_ast_count + 1 > interp->module_ast_cap) {
                interp->module_ast_cap = interp->module_ast_cap < 4 ? 4 : interp->module_ast_cap * 2;
                interp->module_asts = (struct ASTNode **)mrt_realloc(
                    interp->module_asts, sizeof(struct ASTNode *) * interp->module_ast_cap);
                interp->module_sources = (char **)mrt_realloc(
                    interp->module_sources, sizeof(char *) * interp->module_ast_cap);
            }
            interp->module_asts[interp->module_ast_count] = mod_ast;
            interp->module_sources[interp->module_ast_count] = mod_source;
            interp->module_ast_count++;

            parser_free(&parser);
            mrt_free(interp->loading_modules[--interp->loading_count]);

            if (mod_res.status == INTERP_ERROR) {
                mrt_free(resolved_path);
                return mod_res;
            }
            value_release(mod_res.value);

            if (interp->loaded_count + 1 > interp->loaded_cap) {
                interp->loaded_cap = interp->loaded_cap < 4 ? 4 : interp->loaded_cap * 2;
                interp->loaded_modules = (char **)mrt_realloc(
                    interp->loaded_modules, sizeof(char *) * interp->loaded_cap);
            }
            interp->loaded_modules[interp->loaded_count++] = resolved_path;

            return make_ok(value_none());
        }

        case AST_EXPR_STMT: {
            return interpreter_eval_node(interp, node->as.expr_stmt.expression);
        }
    }

    return make_ok(value_none());
}

EvalResult interpreter_interpret(Interpreter *interp, ASTNode *program) {
    if (!program) return make_error();
    return interpreter_eval_node(interp, program);
}

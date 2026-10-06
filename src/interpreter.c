#include "interpreter.h"
#include "builtin.h"
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
}

void interpreter_init(Interpreter *interp, const char *source, const char *filename) {
    interp->globals = env_new(NULL);
    builtin_register_all(interp->globals);
    interp->environment = interp->globals;
    interp->source = source;
    interp->filename = filename;
    interp->had_runtime_error = false;
    interp->owns_globals = true;
}

void interpreter_init_with_env(Interpreter *interp, Environment *globals,
                               const char *source, const char *filename) {
    interp->globals = globals;
    interp->environment = globals;
    interp->source = source;
    interp->filename = filename;
    interp->had_runtime_error = false;
    interp->owns_globals = false;
}

void interpreter_free(Interpreter *interp) {
    if (interp->owns_globals && interp->globals) {
        env_release(interp->globals);
        interp->globals = NULL;
        interp->environment = NULL;
    }
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

static EvalResult make_error(void) {
    EvalResult res;
    res.status = INTERP_ERROR;
    res.value = value_null();
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
            } else {
                report_runtime_error(interp, node, ERR_TYPE,
                                     "cannot compare %s and %s",
                                     value_type_name(l), value_type_name(r));
            }
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
    EvalResult operand_res = interpreter_eval_node(interp, node->as.unary.operand);
    if (operand_res.status != INTERP_OK) return operand_res;

    Value val = operand_res.value;
    TokenType op = node->as.unary.op;

    if (op == TOKEN_MINUS) {
        if (val.type == VAL_INT) {
            value_release(val);
            return make_ok(value_int(-val.as.int_val));
        }
        if (val.type == VAL_FLOAT) {
            value_release(val);
            return make_ok(value_float(-val.as.float_val));
        }
        report_runtime_error(interp, node, ERR_TYPE, "operand must be a number for '-'");
        value_release(val);
        return make_error();
    }

    if (op == TOKEN_NOT) {
        bool truthy = value_is_truthy(val);
        value_release(val);
        return make_ok(value_bool(!truthy));
    }

    report_runtime_error(interp, node, ERR_RUNTIME, "unknown unary operator");
    value_release(val);
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

        EvalResult body_res = interpreter_eval_node(interp, fn->body);

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
                if (body_res.status != INTERP_OK) {
                    return body_res;
                }
                value_release(body_res.value);
            }
            return make_ok(value_null());
        }

        case AST_RETURN: {
            Value ret_val = value_null();
            if (node->as.return_stmt.value) {
                EvalResult res = interpreter_eval_node(interp, node->as.return_stmt.value);
                if (res.status != INTERP_OK) return res;
                ret_val = res.value;
            }
            return make_return(ret_val);
        }

        case AST_EXPR_STMT: {
            return interpreter_eval_node(interp, node->as.expr_stmt.expression);
        }
    }

    return make_ok(value_null());
}

EvalResult interpreter_interpret(Interpreter *interp, ASTNode *program) {
    if (!program) return make_error();
    return interpreter_eval_node(interp, program);
}

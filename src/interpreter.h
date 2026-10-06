#ifndef MRT_INTERPRETER_H
#define MRT_INTERPRETER_H

#include "common.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include "error.h"

typedef enum {
    INTERP_OK,
    INTERP_RETURN,
    INTERP_ERROR
} InterpStatus;

typedef struct {
    InterpStatus status;
    Value value;
} EvalResult;

typedef struct Interpreter {
    Environment *globals;
    Environment *environment;
    const char *source;
    const char *filename;
    bool had_runtime_error;
    bool owns_globals;
} Interpreter;

void interpreter_init(Interpreter *interp, const char *source, const char *filename);
void interpreter_init_with_env(Interpreter *interp, Environment *globals, const char *source, const char *filename);
EvalResult interpreter_interpret(Interpreter *interp, ASTNode *program);
EvalResult interpreter_eval_node(Interpreter *interp, ASTNode *node);
void interpreter_free(Interpreter *interp);

#endif /* MRT_INTERPRETER_H */

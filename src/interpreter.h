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
    INTERP_BREAK,
    INTERP_CONTINUE,
    INTERP_ERROR
} InterpStatus;

typedef struct {
    InterpStatus status;
    Value value;
} EvalResult;

typedef struct CallFrame {
    const char *task_name;
    const char *filename;
    int line;
    int col;
} CallFrame;

#define MAX_CALL_FRAMES 256

typedef struct Interpreter {
    Environment *globals;
    Environment *environment;
    const char *source;
    const char *filename;
    bool had_runtime_error;
    bool owns_globals;

    /* Call stack for diagnostics */
    CallFrame frames[MAX_CALL_FRAMES];
    size_t frame_count;

    /* Module resolution tracking */
    char **loaded_modules;
    size_t loaded_count;
    size_t loaded_cap;
    char **loading_modules;
    size_t loading_count;
    size_t loading_cap;
    struct ASTNode **module_asts;
    char **module_sources;
    size_t module_ast_count;
    size_t module_ast_cap;
} Interpreter;

void interpreter_init(Interpreter *interp, const char *source, const char *filename);
void interpreter_init_with_env(Interpreter *interp, Environment *globals, const char *source, const char *filename);
EvalResult interpreter_interpret(Interpreter *interp, ASTNode *program);
EvalResult interpreter_eval_node(Interpreter *interp, ASTNode *node);
void interpreter_free(Interpreter *interp);

#endif /* MRT_INTERPRETER_H */

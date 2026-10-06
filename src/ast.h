#ifndef MRT_AST_H
#define MRT_AST_H

#include "common.h"
#include "token.h"

typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_BINARY,
    AST_UNARY,
    AST_FUNCTION_DECL,
    AST_FUNCTION_CALL,
    AST_IF,
    AST_WHILE,
    AST_RETURN,
    AST_EXPR_STMT
} ASTNodeType;

typedef enum {
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_BOOL,
    LITERAL_NULL
} LiteralType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    ASTNodeType type;
    int line;
    int column;

    union {
        /* AST_PROGRAM, AST_BLOCK */
        struct {
            ASTNode **statements;
            size_t count;
            size_t capacity;
        } block;

        /* AST_VAR_DECL */
        struct {
            char *name;
            ASTNode *init;
        } var_decl;

        /* AST_ASSIGN */
        struct {
            char *name;
            ASTNode *value;
        } assign;

        /* AST_LITERAL */
        struct {
            LiteralType lit_type;
            union {
                int64_t int_val;
                double float_val;
                char *string_val;
                bool bool_val;
            } as;
        } literal;

        /* AST_IDENTIFIER */
        struct {
            char *name;
        } identifier;

        /* AST_BINARY */
        struct {
            TokenType op;
            ASTNode *left;
            ASTNode *right;
        } binary;

        /* AST_UNARY */
        struct {
            TokenType op;
            ASTNode *operand;
        } unary;

        /* AST_FUNCTION_DECL */
        struct {
            char *name;
            char **params;
            size_t param_count;
            ASTNode *body; /* AST_BLOCK */
        } func_decl;

        /* AST_FUNCTION_CALL */
        struct {
            ASTNode *callee;
            ASTNode **args;
            size_t arg_count;
        } func_call;

        /* AST_IF */
        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch; /* can be NULL */
        } if_stmt;

        /* AST_WHILE */
        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_stmt;

        /* AST_RETURN */
        struct {
            ASTNode *value; /* can be NULL */
        } return_stmt;

        /* AST_EXPR_STMT */
        struct {
            ASTNode *expression;
        } expr_stmt;
    } as;
};

/* AST Constructors */
ASTNode *ast_new_program(int line, int col);
void ast_program_add_stmt(ASTNode *prog, ASTNode *stmt);

ASTNode *ast_new_block(int line, int col);
void ast_block_add_stmt(ASTNode *block, ASTNode *stmt);

ASTNode *ast_new_var_decl(char *name, ASTNode *init, int line, int col);
ASTNode *ast_new_assign(char *name, ASTNode *value, int line, int col);

ASTNode *ast_new_literal_int(int64_t val, int line, int col);
ASTNode *ast_new_literal_float(double val, int line, int col);
ASTNode *ast_new_literal_string(char *val, int line, int col);
ASTNode *ast_new_literal_bool(bool val, int line, int col);
ASTNode *ast_new_literal_null(int line, int col);

ASTNode *ast_new_identifier(char *name, int line, int col);
ASTNode *ast_new_binary(TokenType op, ASTNode *left, ASTNode *right, int line, int col);
ASTNode *ast_new_unary(TokenType op, ASTNode *operand, int line, int col);

ASTNode *ast_new_func_decl(char *name, char **params, size_t param_count, ASTNode *body, int line, int col);
ASTNode *ast_new_func_call(ASTNode *callee, ASTNode **args, size_t arg_count, int line, int col);

ASTNode *ast_new_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, int line, int col);
ASTNode *ast_new_while(ASTNode *condition, ASTNode *body, int line, int col);
ASTNode *ast_new_return(ASTNode *value, int line, int col);
ASTNode *ast_new_expr_stmt(ASTNode *expr, int line, int col);

/* AST Destructor */
void ast_free(ASTNode *node);

/* AST Pretty Printer */
void ast_print(const ASTNode *node);

#endif /* MRT_AST_H */

#ifndef MRT_TOKEN_H
#define MRT_TOKEN_H

#include "common.h"

typedef enum {
    /* Literals */
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_YES,              /* yes */
    TOKEN_NO,               /* no */
    TOKEN_NONE,             /* none */

    /* Identifiers */
    TOKEN_IDENTIFIER,

    /* Keywords */
    TOKEN_VAR,              /* var */
    TOKEN_TASK,             /* task */
    TOKEN_GIVE,             /* give */
    TOKEN_SAY,              /* say */
    TOKEN_WHEN,             /* when */
    TOKEN_OTHERWISE,        /* otherwise */
    TOKEN_REPEAT,           /* repeat */
    TOKEN_BREAK,            /* break */
    TOKEN_CONTINUE,         /* continue */
    TOKEN_AND,              /* and */
    TOKEN_OR,               /* or */
    TOKEN_NOT,              /* not */

    /* Reserved future keywords */
    TOKEN_USE,
    TOKEN_FROM,
    TOKEN_AS,
    TOKEN_EACH,
    TOKEN_IN,
    TOKEN_TYPE,

    /* Operators */
    TOKEN_PLUS,             /* + */
    TOKEN_MINUS,            /* - */
    TOKEN_STAR,             /* * */
    TOKEN_SLASH,            /* / */
    TOKEN_PERCENT,          /* % */
    TOKEN_EQUAL,            /* = */
    TOKEN_EQUAL_EQUAL,      /* == */
    TOKEN_BANG_EQUAL,       /* != */
    TOKEN_LESS,             /* < */
    TOKEN_LESS_EQUAL,       /* <= */
    TOKEN_GREATER,          /* > */
    TOKEN_GREATER_EQUAL,    /* >= */

    /* Punctuation */
    TOKEN_LPAREN,           /* ( */
    TOKEN_RPAREN,           /* ) */
    TOKEN_LBRACE,           /* { */
    TOKEN_RBRACE,           /* } */
    TOKEN_LBRACKET,         /* [ */
    TOKEN_RBRACKET,         /* ] */
    TOKEN_COLON,            /* : */
    TOKEN_COMMA,            /* , */
    TOKEN_SEMICOLON,        /* ; */
    TOKEN_NEWLINE,          /* \n */

    /* Special */
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
    union {
        int64_t int_val;
        double float_val;
        char *string_val;
    } as;
} Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenArray;

const char *token_type_name(TokenType type);
Token token_make(TokenType type, const char *lexeme, int line, int col);
void token_free_members(Token *token);

void token_array_init(TokenArray *array);
void token_array_push(TokenArray *array, Token token);
void token_array_free(TokenArray *array);
void token_print(const Token *token);

#endif /* MRT_TOKEN_H */

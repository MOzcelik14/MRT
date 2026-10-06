#ifndef MRT_LEXER_H
#define MRT_LEXER_H

#include "token.h"

typedef struct {
    const char *source;
    const char *filename;
    const char *start;
    const char *current;
    int line;
    int column;
    int start_column;
    bool had_error;
} Lexer;

void lexer_init(Lexer *lexer, const char *source, const char *filename);
Token lexer_next_token(Lexer *lexer);
TokenArray lexer_tokenize_all(Lexer *lexer);

#endif /* MRT_LEXER_H */

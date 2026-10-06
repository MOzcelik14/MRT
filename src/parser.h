#ifndef MRT_PARSER_H
#define MRT_PARSER_H

#include "common.h"
#include "token.h"
#include "ast.h"

typedef struct {
    TokenArray tokens;
    size_t current;
    const char *source;
    const char *filename;
    bool had_error;
    bool panic_mode;
} Parser;

void parser_init(Parser *parser, TokenArray tokens, const char *source, const char *filename);
ASTNode *parser_parse(Parser *parser);
void parser_free(Parser *parser);

#endif /* MRT_PARSER_H */

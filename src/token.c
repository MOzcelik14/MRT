#include "token.h"

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_INT:           return "TOKEN_INT";
        case TOKEN_FLOAT:         return "TOKEN_FLOAT";
        case TOKEN_STRING:        return "TOKEN_STRING";
        case TOKEN_TRUE:          return "TOKEN_TRUE";
        case TOKEN_FALSE:         return "TOKEN_FALSE";
        case TOKEN_NULL:          return "TOKEN_NULL";
        case TOKEN_IDENTIFIER:    return "TOKEN_IDENTIFIER";
        case TOKEN_LET:           return "TOKEN_LET";
        case TOKEN_FN:            return "TOKEN_FN";
        case TOKEN_RETURN:        return "TOKEN_RETURN";
        case TOKEN_IF:            return "TOKEN_IF";
        case TOKEN_ELSE:          return "TOKEN_ELSE";
        case TOKEN_WHILE:         return "TOKEN_WHILE";
        case TOKEN_AND:           return "TOKEN_AND";
        case TOKEN_OR:            return "TOKEN_OR";
        case TOKEN_NOT:           return "TOKEN_NOT";
        case TOKEN_PLUS:          return "TOKEN_PLUS";
        case TOKEN_MINUS:         return "TOKEN_MINUS";
        case TOKEN_STAR:          return "TOKEN_STAR";
        case TOKEN_SLASH:         return "TOKEN_SLASH";
        case TOKEN_PERCENT:       return "TOKEN_PERCENT";
        case TOKEN_EQUAL:         return "TOKEN_EQUAL";
        case TOKEN_EQUAL_EQUAL:   return "TOKEN_EQUAL_EQUAL";
        case TOKEN_BANG_EQUAL:    return "TOKEN_BANG_EQUAL";
        case TOKEN_LESS:          return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL:    return "TOKEN_LESS_EQUAL";
        case TOKEN_GREATER:       return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LPAREN:        return "TOKEN_LPAREN";
        case TOKEN_RPAREN:        return "TOKEN_RPAREN";
        case TOKEN_LBRACE:        return "TOKEN_LBRACE";
        case TOKEN_RBRACE:        return "TOKEN_RBRACE";
        case TOKEN_COMMA:         return "TOKEN_COMMA";
        case TOKEN_SEMICOLON:     return "TOKEN_SEMICOLON";
        case TOKEN_NEWLINE:       return "TOKEN_NEWLINE";
        case TOKEN_EOF:           return "TOKEN_EOF";
        case TOKEN_ERROR:         return "TOKEN_ERROR";
        default:                  return "TOKEN_UNKNOWN";
    }
}

Token token_make(TokenType type, const char *lexeme, int line, int col) {
    Token token;
    token.type = type;
    token.lexeme = lexeme ? mrt_strdup(lexeme) : NULL;
    token.line = line;
    token.column = col;
    token.as.int_val = 0;
    return token;
}

void token_free_members(Token *token) {
    if (!token) return;
    if (token->lexeme) {
        mrt_free(token->lexeme);
        token->lexeme = NULL;
    }
    if (token->type == TOKEN_STRING && token->as.string_val) {
        mrt_free(token->as.string_val);
        token->as.string_val = NULL;
    }
}

void token_array_init(TokenArray *array) {
    array->tokens = NULL;
    array->count = 0;
    array->capacity = 0;
}

void token_array_push(TokenArray *array, Token token) {
    if (array->count + 1 > array->capacity) {
        size_t new_cap = array->capacity < 8 ? 8 : array->capacity * 2;
        array->tokens = (Token *)mrt_realloc(array->tokens, new_cap * sizeof(Token));
        array->capacity = new_cap;
    }
    array->tokens[array->count++] = token;
}

void token_array_free(TokenArray *array) {
    if (!array) return;
    for (size_t i = 0; i < array->count; i++) {
        token_free_members(&array->tokens[i]);
    }
    mrt_free(array->tokens);
    array->tokens = NULL;
    array->count = 0;
    array->capacity = 0;
}

void token_print(const Token *token) {
    if (!token) return;
    printf("%-20s (line %3d, col %3d) '%s'\n",
           token_type_name(token->type),
           token->line,
           token->column,
           token->lexeme ? token->lexeme : "");
}

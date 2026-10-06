#include "lexer.h"
#include "error.h"

void lexer_init(Lexer *lexer, const char *source, const char *filename) {
    lexer->source = source;
    lexer->filename = filename;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start_column = 1;
    lexer->had_error = false;
}

static bool is_at_end(Lexer *lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer *lexer) {
    char c = *lexer->current++;
    lexer->column++;
    return c;
}

static char peek(Lexer *lexer) {
    return *lexer->current;
}

static char peek_next(Lexer *lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static bool match(Lexer *lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    lexer->current++;
    lexer->column++;
    return true;
}

static void skip_whitespace_and_comments(Lexer *lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // Single-line comment: skip until newline or end of file
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token make_token(Lexer *lexer, TokenType type) {
    size_t length = (size_t)(lexer->current - lexer->start);
    char *lexeme = mrt_strndup(lexer->start, length);
    Token token;
    token.type = type;
    token.lexeme = lexeme;
    token.line = lexer->line;
    token.column = lexer->start_column;
    token.as.int_val = 0;
    return token;
}

static Token error_token(Lexer *lexer, const char *message) {
    lexer->had_error = true;
    mrt_report_error(lexer->filename, lexer->source,
                     lexer->line, lexer->start_column,
                     ERR_SYNTAX, "%s", message);
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = mrt_strdup(message);
    token.line = lexer->line;
    token.column = lexer->start_column;
    token.as.int_val = 0;
    return token;
}

static TokenType check_keyword(const char *text, size_t length) {
    switch (length) {
        case 2:
            if (text[0] == 'f' && text[1] == 'n') return TOKEN_FN;
            if (text[0] == 'i' && text[1] == 'f') return TOKEN_IF;
            if (text[0] == 'o' && text[1] == 'r') return TOKEN_OR;
            break;
        case 3:
            if (memcmp(text, "let", 3) == 0) return TOKEN_LET;
            if (memcmp(text, "and", 3) == 0) return TOKEN_AND;
            if (memcmp(text, "not", 3) == 0) return TOKEN_NOT;
            break;
        case 4:
            if (memcmp(text, "else", 4) == 0) return TOKEN_ELSE;
            if (memcmp(text, "true", 4) == 0) return TOKEN_TRUE;
            if (memcmp(text, "null", 4) == 0) return TOKEN_NULL;
            break;
        case 5:
            if (memcmp(text, "while", 5) == 0) return TOKEN_WHILE;
            if (memcmp(text, "false", 5) == 0) return TOKEN_FALSE;
            break;
        case 6:
            if (memcmp(text, "return", 6) == 0) return TOKEN_RETURN;
            break;
    }
    return TOKEN_IDENTIFIER;
}

static Token scan_identifier(Lexer *lexer) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    size_t length = (size_t)(lexer->current - lexer->start);
    TokenType type = check_keyword(lexer->start, length);
    return make_token(lexer, type);
}

static Token scan_number(Lexer *lexer) {
    bool is_float = false;

    while (isdigit(peek(lexer))) {
        advance(lexer);
    }

    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        is_float = true;
        advance(lexer); // consume '.'
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
    }

    Token token;
    if (is_float) {
        token = make_token(lexer, TOKEN_FLOAT);
        token.as.float_val = strtod(token.lexeme, NULL);
    } else {
        token = make_token(lexer, TOKEN_INT);
        token.as.int_val = strtoll(token.lexeme, NULL, 10);
    }
    return token;
}

static Token scan_string(Lexer *lexer) {
    // Current is right after the opening '"'
    size_t cap = 32;
    size_t len = 0;
    char *buf = (char *)mrt_malloc(cap);

    while (!is_at_end(lexer) && peek(lexer) != '"') {
        char c = advance(lexer);
        if (c == '\n') {
            lexer->line++;
            lexer->column = 1;
        }

        if (c == '\\') {
            if (is_at_end(lexer)) {
                mrt_free(buf);
                return error_token(lexer, "unterminated string escape");
            }
            char esc = advance(lexer);
            switch (esc) {
                case 'n':  c = '\n'; break;
                case 't':  c = '\t'; break;
                case 'r':  c = '\r'; break;
                case '"':  c = '"';  break;
                case '\\': c = '\\'; break;
                case '0':  c = '\0'; break;
                default:
                    mrt_free(buf);
                    return error_token(lexer, "unknown escape sequence in string");
            }
        }

        if (len + 1 >= cap) {
            cap *= 2;
            buf = (char *)mrt_realloc(buf, cap);
        }
        buf[len++] = c;
    }

    if (is_at_end(lexer)) {
        mrt_free(buf);
        return error_token(lexer, "unterminated string literal");
    }

    // Consume the closing '"'
    advance(lexer);
    buf[len] = '\0';

    Token token = make_token(lexer, TOKEN_STRING);
    token.as.string_val = buf;
    return token;
}

Token lexer_next_token(Lexer *lexer) {
    skip_whitespace_and_comments(lexer);

    lexer->start = lexer->current;
    lexer->start_column = lexer->column;

    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);

    if (c == '\n') {
        Token token = make_token(lexer, TOKEN_NEWLINE);
        lexer->line++;
        lexer->column = 1;
        return token;
    }

    if (isalpha(c) || c == '_') {
        return scan_identifier(lexer);
    }

    if (isdigit(c)) {
        return scan_number(lexer);
    }

    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '-': return make_token(lexer, TOKEN_MINUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, TOKEN_SLASH);
        case '%': return make_token(lexer, TOKEN_PERCENT);
        case '=':
            return match(lexer, '=') ? make_token(lexer, TOKEN_EQUAL_EQUAL)
                                     : make_token(lexer, TOKEN_EQUAL);
        case '!':
            if (match(lexer, '=')) {
                return make_token(lexer, TOKEN_BANG_EQUAL);
            }
            return error_token(lexer, "unexpected character '!', did you mean 'not' or '!='?");
        case '<':
            return match(lexer, '=') ? make_token(lexer, TOKEN_LESS_EQUAL)
                                     : make_token(lexer, TOKEN_LESS);
        case '>':
            return match(lexer, '=') ? make_token(lexer, TOKEN_GREATER_EQUAL)
                                     : make_token(lexer, TOKEN_GREATER);
        case '"':
            return scan_string(lexer);
        default: {
            char msg[64];
            snprintf(msg, sizeof(msg), "unexpected character '%c'", c);
            return error_token(lexer, msg);
        }
    }
}

TokenArray lexer_tokenize_all(Lexer *lexer) {
    TokenArray array;
    token_array_init(&array);

    for (;;) {
        Token token = lexer_next_token(lexer);
        token_array_push(&array, token);
        if (token.type == TOKEN_EOF || token.type == TOKEN_ERROR) {
            break;
        }
    }

    return array;
}

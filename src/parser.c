#include "parser.h"
#include "error.h"

static Token peek(Parser *parser) {
    if (parser->current >= parser->tokens.count) {
        Token eof_tok = token_make(TOKEN_EOF, "", 0, 0);
        return eof_tok;
    }
    return parser->tokens.tokens[parser->current];
}

static Token previous(Parser *parser) {
    if (parser->current == 0) return parser->tokens.tokens[0];
    return parser->tokens.tokens[parser->current - 1];
}

static bool is_at_end(Parser *parser) {
    return peek(parser).type == TOKEN_EOF;
}

static Token advance(Parser *parser) {
    if (!is_at_end(parser)) {
        parser->current++;
    }
    return previous(parser);
}

static bool check(Parser *parser, TokenType type) {
    return peek(parser).type == type;
}

static bool match(Parser *parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

static void report_parser_error(Parser *parser, Token token, const char *fmt, ...) {
    if (parser->panic_mode) return;
    parser->had_error = true;
    parser->panic_mode = true;

    char message[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    mrt_report_error(parser->filename, parser->source,
                     token.line, token.column,
                     ERR_SYNTAX, "%s", message);
}

static Token consume(Parser *parser, TokenType type, const char *message) {
    if (check(parser, type)) return advance(parser);
    report_parser_error(parser, peek(parser), "%s", message);
    return peek(parser);
}

static void skip_newlines(Parser *parser) {
    while (check(parser, TOKEN_NEWLINE)) {
        advance(parser);
    }
}

static void synchronize(Parser *parser) {
    parser->panic_mode = false;

    while (!is_at_end(parser)) {
        if (previous(parser).type == TOKEN_SEMICOLON || previous(parser).type == TOKEN_NEWLINE) {
            return;
        }

        switch (peek(parser).type) {
            case TOKEN_TASK:
            case TOKEN_VAR:
            case TOKEN_WHEN:
            case TOKEN_OTHERWISE:
            case TOKEN_REPEAT:
            case TOKEN_GIVE:
            case TOKEN_SAY:
            case TOKEN_BREAK:
            case TOKEN_CONTINUE:
            case TOKEN_RBRACE:
                return;
            default:
                advance(parser);
                break;
        }
    }
}

/* Forward declarations */
static ASTNode *parse_declaration(Parser *parser);
static ASTNode *parse_statement(Parser *parser);
static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_block(Parser *parser);

/* Expression Parsing with Precedence Climbing */
static ASTNode *parse_assignment(Parser *parser);
static ASTNode *parse_or(Parser *parser);
static ASTNode *parse_and(Parser *parser);
static ASTNode *parse_equality(Parser *parser);
static ASTNode *parse_comparison(Parser *parser);
static ASTNode *parse_term(Parser *parser);
static ASTNode *parse_factor(Parser *parser);
static ASTNode *parse_unary(Parser *parser);
static ASTNode *parse_call(Parser *parser);
static ASTNode *parse_primary(Parser *parser);

static ASTNode *parse_expression(Parser *parser) {
    return parse_assignment(parser);
}

static ASTNode *parse_assignment(Parser *parser) {
    ASTNode *expr = parse_or(parser);

    if (match(parser, TOKEN_EQUAL)) {
        Token equals = previous(parser);
        skip_newlines(parser);
        ASTNode *value = parse_assignment(parser);

        if (expr && expr->type == AST_IDENTIFIER) {
            char *name = mrt_strdup(expr->as.identifier.name);
            int line = expr->line;
            int col = expr->column;
            ast_free(expr);
            return ast_new_assign(name, value, line, col);
        }

        report_parser_error(parser, equals, "invalid assignment target");
        ast_free(expr);
        ast_free(value);
        return NULL;
    }

    return expr;
}

static ASTNode *parse_or(Parser *parser) {
    ASTNode *expr = parse_and(parser);

    while (match(parser, TOKEN_OR)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *right = parse_and(parser);
        expr = ast_new_binary(op.type, expr, right, op.line, op.column);
    }

    return expr;
}

static ASTNode *parse_and(Parser *parser) {
    ASTNode *expr = parse_equality(parser);

    while (match(parser, TOKEN_AND)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *right = parse_equality(parser);
        expr = ast_new_binary(op.type, expr, right, op.line, op.column);
    }

    return expr;
}

static ASTNode *parse_equality(Parser *parser) {
    ASTNode *expr = parse_comparison(parser);

    while (match(parser, TOKEN_EQUAL_EQUAL) || match(parser, TOKEN_BANG_EQUAL)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *right = parse_comparison(parser);
        expr = ast_new_binary(op.type, expr, right, op.line, op.column);
    }

    return expr;
}

static ASTNode *parse_comparison(Parser *parser) {
    ASTNode *expr = parse_term(parser);

    while (match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL) ||
           match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *right = parse_term(parser);
        expr = ast_new_binary(op.type, expr, right, op.line, op.column);
    }

    return expr;
}

static ASTNode *parse_term(Parser *parser) {
    ASTNode *expr = parse_factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *right = parse_factor(parser);
        expr = ast_new_binary(op.type, expr, right, op.line, op.column);
    }

    return expr;
}

static ASTNode *parse_factor(Parser *parser) {
    ASTNode *expr = parse_unary(parser);

    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) || match(parser, TOKEN_PERCENT)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *right = parse_unary(parser);
        expr = ast_new_binary(op.type, expr, right, op.line, op.column);
    }

    return expr;
}

static ASTNode *parse_unary(Parser *parser) {
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_NOT)) {
        Token op = previous(parser);
        skip_newlines(parser);
        ASTNode *operand = parse_unary(parser);
        return ast_new_unary(op.type, operand, op.line, op.column);
    }

    return parse_call(parser);
}

static ASTNode *finish_call(Parser *parser, ASTNode *callee) {
    ASTNode **args = NULL;
    size_t count = 0;
    size_t cap = 0;

    skip_newlines(parser);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            skip_newlines(parser);
            ASTNode *arg = parse_expression(parser);
            if (count + 1 > cap) {
                cap = cap < 4 ? 4 : cap * 2;
                args = (ASTNode **)mrt_realloc(args, cap * sizeof(ASTNode *));
            }
            args[count++] = arg;
            skip_newlines(parser);
        } while (match(parser, TOKEN_COMMA));
    }

    skip_newlines(parser);
    consume(parser, TOKEN_RPAREN, "expected ')' after arguments");

    return ast_new_func_call(callee, args, count, callee->line, callee->column);
}

static ASTNode *parse_call(Parser *parser) {
    ASTNode *expr = parse_primary(parser);

    for (;;) {
        if (match(parser, TOKEN_LPAREN)) {
            expr = finish_call(parser, expr);
        } else {
            break;
        }
    }

    return expr;
}

static ASTNode *parse_primary(Parser *parser) {
    if (match(parser, TOKEN_INT)) {
        Token tok = previous(parser);
        return ast_new_literal_int(tok.as.int_val, tok.line, tok.column);
    }

    if (match(parser, TOKEN_FLOAT)) {
        Token tok = previous(parser);
        return ast_new_literal_float(tok.as.float_val, tok.line, tok.column);
    }

    if (match(parser, TOKEN_STRING)) {
        Token tok = previous(parser);
        char *str = tok.as.string_val ? mrt_strdup(tok.as.string_val) : mrt_strdup("");
        return ast_new_literal_string(str, tok.line, tok.column);
    }

    if (match(parser, TOKEN_YES)) {
        Token tok = previous(parser);
        return ast_new_literal_bool(true, tok.line, tok.column);
    }

    if (match(parser, TOKEN_NO)) {
        Token tok = previous(parser);
        return ast_new_literal_bool(false, tok.line, tok.column);
    }

    if (match(parser, TOKEN_NONE)) {
        Token tok = previous(parser);
        return ast_new_literal_none(tok.line, tok.column);
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        Token tok = previous(parser);
        return ast_new_identifier(mrt_strdup(tok.lexeme), tok.line, tok.column);
    }

    if (match(parser, TOKEN_LPAREN)) {
        skip_newlines(parser);
        ASTNode *expr = parse_expression(parser);
        skip_newlines(parser);
        consume(parser, TOKEN_RPAREN, "expected ')' after grouped expression");
        return expr;
    }

    Token tok = peek(parser);
    report_parser_error(parser, tok, "expected expression");
    advance(parser);
    return NULL;
}

static void consume_statement_separator(Parser *parser) {
    if (match(parser, TOKEN_SEMICOLON) || match(parser, TOKEN_NEWLINE)) {
        while (match(parser, TOKEN_SEMICOLON) || match(parser, TOKEN_NEWLINE));
        return;
    }

    if (check(parser, TOKEN_RBRACE) || check(parser, TOKEN_EOF)) {
        return;
    }

    report_parser_error(parser, peek(parser), "expected newline or ';' after statement");
}

static ASTNode *parse_var_decl(Parser *parser) {
    Token var_tok = previous(parser);
    Token name_tok = consume(parser, TOKEN_IDENTIFIER, "expected variable name after 'var'");
    if (parser->panic_mode) return NULL;

    char *name = mrt_strdup(name_tok.lexeme);
    ASTNode *init = NULL;

    if (match(parser, TOKEN_EQUAL)) {
        skip_newlines(parser);
        init = parse_expression(parser);
    } else {
        init = ast_new_literal_none(var_tok.line, var_tok.column);
    }

    consume_statement_separator(parser);
    if (parser->panic_mode) {
        mrt_free(name);
        ast_free(init);
        return NULL;
    }
    return ast_new_var_decl(name, init, var_tok.line, var_tok.column);
}

static ASTNode *parse_block_body(Parser *parser, int line, int col) {
    ASTNode *block = ast_new_block(line, col);

    skip_newlines(parser);
    while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
        ASTNode *stmt = parse_declaration(parser);
        if (stmt) {
            ast_block_add_stmt(block, stmt);
        }
        skip_newlines(parser);
    }

    consume(parser, TOKEN_RBRACE, "expected '}' after block");
    return block;
}

static ASTNode *parse_block(Parser *parser) {
    Token tok = consume(parser, TOKEN_LBRACE, "expected '{' to start block");
    return parse_block_body(parser, tok.line, tok.column);
}

static ASTNode *parse_func_decl(Parser *parser) {
    Token task_tok = previous(parser);
    Token name_tok = consume(parser, TOKEN_IDENTIFIER, "expected function name after 'task'");
    if (parser->panic_mode) return NULL;

    consume(parser, TOKEN_LPAREN, "expected '(' after function name");

    char **params = NULL;
    size_t param_count = 0;
    size_t param_cap = 0;

    skip_newlines(parser);
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            skip_newlines(parser);
            Token p_tok = consume(parser, TOKEN_IDENTIFIER, "expected parameter name");
            if (param_count + 1 > param_cap) {
                param_cap = param_cap < 4 ? 4 : param_cap * 2;
                params = (char **)mrt_realloc(params, param_cap * sizeof(char *));
            }
            params[param_count++] = mrt_strdup(p_tok.lexeme);
            skip_newlines(parser);
        } while (match(parser, TOKEN_COMMA));
    }

    skip_newlines(parser);
    consume(parser, TOKEN_RPAREN, "expected ')' after parameters");
    skip_newlines(parser);

    Token lbrace = consume(parser, TOKEN_LBRACE, "expected '{' before function body");
    ASTNode *body = parse_block_body(parser, lbrace.line, lbrace.column);

    while (match(parser, TOKEN_NEWLINE) || match(parser, TOKEN_SEMICOLON));

    return ast_new_func_decl(mrt_strdup(name_tok.lexeme), params, param_count,
                             body, task_tok.line, task_tok.column);
}

static ASTNode *parse_when(Parser *parser) {
    Token when_tok = previous(parser);
    skip_newlines(parser);

    ASTNode *condition = parse_expression(parser);
    skip_newlines(parser);

    Token lbrace = consume(parser, TOKEN_LBRACE, "expected '{' after condition");
    ASTNode *then_branch = parse_block_body(parser, lbrace.line, lbrace.column);

    ASTNode *else_branch = NULL;
    skip_newlines(parser);
    if (match(parser, TOKEN_OTHERWISE)) {
        skip_newlines(parser);
        if (match(parser, TOKEN_WHEN)) {
            else_branch = parse_when(parser);
        } else {
            Token else_lbrace = consume(parser, TOKEN_LBRACE, "expected '{' after 'otherwise'");
            else_branch = parse_block_body(parser, else_lbrace.line, else_lbrace.column);
        }
    }

    while (match(parser, TOKEN_NEWLINE) || match(parser, TOKEN_SEMICOLON));

    return ast_new_if(condition, then_branch, else_branch, when_tok.line, when_tok.column);
}

static ASTNode *parse_repeat(Parser *parser) {
    Token rep_tok = previous(parser);
    skip_newlines(parser);

    ASTNode *condition = parse_expression(parser);
    skip_newlines(parser);

    Token lbrace = consume(parser, TOKEN_LBRACE, "expected '{' after repeat condition");
    ASTNode *body = parse_block_body(parser, lbrace.line, lbrace.column);

    while (match(parser, TOKEN_NEWLINE) || match(parser, TOKEN_SEMICOLON));

    return ast_new_while(condition, body, rep_tok.line, rep_tok.column);
}

static ASTNode *parse_give(Parser *parser) {
    Token give_tok = previous(parser);
    ASTNode *value = NULL;

    if (!check(parser, TOKEN_NEWLINE) && !check(parser, TOKEN_SEMICOLON) &&
        !check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        value = parse_expression(parser);
    }

    consume_statement_separator(parser);
    return ast_new_return(value, give_tok.line, give_tok.column);
}

static ASTNode *parse_say(Parser *parser) {
    Token say_tok = previous(parser);
    skip_newlines(parser);
    ASTNode *value = parse_expression(parser);
    consume_statement_separator(parser);
    return ast_new_say(value, say_tok.line, say_tok.column);
}

static ASTNode *parse_break(Parser *parser) {
    Token tok = previous(parser);
    consume_statement_separator(parser);
    return ast_new_break(tok.line, tok.column);
}

static ASTNode *parse_continue(Parser *parser) {
    Token tok = previous(parser);
    consume_statement_separator(parser);
    return ast_new_continue(tok.line, tok.column);
}

static ASTNode *parse_expr_statement(Parser *parser) {
    int line = peek(parser).line;
    int col = peek(parser).column;
    ASTNode *expr = parse_expression(parser);
    consume_statement_separator(parser);
    return ast_new_expr_stmt(expr, line, col);
}

static ASTNode *parse_statement(Parser *parser) {
    if (match(parser, TOKEN_WHEN)) return parse_when(parser);
    if (match(parser, TOKEN_REPEAT)) return parse_repeat(parser);
    if (match(parser, TOKEN_GIVE)) return parse_give(parser);
    if (match(parser, TOKEN_SAY)) return parse_say(parser);
    if (match(parser, TOKEN_BREAK)) return parse_break(parser);
    if (match(parser, TOKEN_CONTINUE)) return parse_continue(parser);
    if (check(parser, TOKEN_LBRACE)) return parse_block(parser);

    return parse_expr_statement(parser);
}

static ASTNode *parse_declaration(Parser *parser) {
    if (match(parser, TOKEN_VAR)) return parse_var_decl(parser);
    if (match(parser, TOKEN_TASK)) return parse_func_decl(parser);

    ASTNode *stmt = parse_statement(parser);
    if (parser->panic_mode) {
        ast_free(stmt);
        synchronize(parser);
        return NULL;
    }
    return stmt;
}

void parser_init(Parser *parser, TokenArray tokens, const char *source, const char *filename) {
    parser->tokens = tokens;
    parser->current = 0;
    parser->source = source;
    parser->filename = filename;
    parser->had_error = false;
    parser->panic_mode = false;
}

ASTNode *parser_parse(Parser *parser) {
    ASTNode *program = ast_new_program(1, 1);

    skip_newlines(parser);
    while (!is_at_end(parser)) {
        ASTNode *stmt = parse_declaration(parser);
        if (stmt) {
            ast_program_add_stmt(program, stmt);
        }
        skip_newlines(parser);
    }

    if (parser->had_error) {
        ast_free(program);
        return NULL;
    }

    return program;
}

void parser_free(Parser *parser) {
    token_array_free(&parser->tokens);
}

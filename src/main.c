#include "common.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "interpreter.h"
#include "builtin.h"

static void print_version(void) {
    printf("%s %s\n", MRT_NAME, MRT_VERSION);
}

static void print_help(void) {
    printf("MRT Programming Language (%s)\n\n", MRT_VERSION);
    printf("Usage:\n");
    printf("  mrt [options] [file.mrt]\n\n");
    printf("Options:\n");
    printf("  --tokens <file.mrt>   Tokenize file and print token stream\n");
    printf("  --ast <file.mrt>      Parse file and print abstract syntax tree\n");
    printf("  --version             Display version information\n");
    printf("  --help                Display this help message\n\n");
    printf("If no file is provided, MRT starts in interactive REPL mode.\n");
}

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Error: could not open file '%s'\n", path);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 0) {
        fprintf(stderr, "Error: could not read file '%s'\n", path);
        fclose(file);
        return NULL;
    }

    char *buffer = (char *)mrt_malloc((size_t)size + 1);
    size_t bytes_read = fread(buffer, sizeof(char), (size_t)size, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

static int run_tokens(const char *filename) {
    char *source = read_file(filename);
    if (!source) return 1;

    Lexer lexer;
    lexer_init(&lexer, source, filename);
    TokenArray tokens = lexer_tokenize_all(&lexer);

    for (size_t i = 0; i < tokens.count; i++) {
        token_print(&tokens.tokens[i]);
    }

    int exit_code = lexer.had_error ? 1 : 0;
    token_array_free(&tokens);
    mrt_free(source);
    return exit_code;
}

static int run_ast(const char *filename) {
    char *source = read_file(filename);
    if (!source) return 1;

    Lexer lexer;
    lexer_init(&lexer, source, filename);
    TokenArray tokens = lexer_tokenize_all(&lexer);

    if (lexer.had_error) {
        token_array_free(&tokens);
        mrt_free(source);
        return 1;
    }

    Parser parser;
    parser_init(&parser, tokens, source, filename);
    ASTNode *ast = parser_parse(&parser);

    int exit_code = 0;
    if (ast) {
        ast_print(ast);
        ast_free(ast);
    } else {
        exit_code = 1;
    }

    parser_free(&parser);
    mrt_free(source);
    return exit_code;
}

static int run_file(const char *filename) {
    char *source = read_file(filename);
    if (!source) return 1;

    Lexer lexer;
    lexer_init(&lexer, source, filename);
    TokenArray tokens = lexer_tokenize_all(&lexer);

    if (lexer.had_error) {
        token_array_free(&tokens);
        mrt_free(source);
        return 1;
    }

    Parser parser;
    parser_init(&parser, tokens, source, filename);
    ASTNode *ast = parser_parse(&parser);

    if (!ast) {
        parser_free(&parser);
        mrt_free(source);
        return 1;
    }

    Interpreter interp;
    interpreter_init(&interp, source, filename);

    EvalResult res = interpreter_interpret(&interp, ast);
    value_release(res.value);

    int exit_code = (res.status == INTERP_ERROR || interp.had_runtime_error) ? 1 : 0;

    interpreter_free(&interp);
    ast_free(ast);
    parser_free(&parser);
    mrt_free(source);
    return exit_code;
}

static void run_repl(void) {
    print_version();
    printf("Interactive interpreter\n\n");

    Environment *globals = env_new(NULL);
    builtin_register_all(globals);

    char line[4096];

    for (;;) {
        printf(">>> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        /* Skip empty lines */
        const char *p = line;
        while (*p && isspace(*p)) p++;
        if (*p == '\0') continue;

        Lexer lexer;
        lexer_init(&lexer, line, "<stdin>");
        TokenArray tokens = lexer_tokenize_all(&lexer);

        if (lexer.had_error) {
            token_array_free(&tokens);
            continue;
        }

        Parser parser;
        parser_init(&parser, tokens, line, "<stdin>");
        ASTNode *ast = parser_parse(&parser);

        if (!ast) {
            parser_free(&parser);
            continue;
        }

        Interpreter interp;
        interpreter_init_with_env(&interp, globals, line, "<stdin>");

        /* If user typed a single expression, print its evaluation result */
        bool is_expr = (ast->type == AST_PROGRAM &&
                        ast->as.block.count == 1 &&
                        ast->as.block.statements[0]->type == AST_EXPR_STMT);

        EvalResult res = interpreter_interpret(&interp, ast);

        if (res.status == INTERP_OK) {
            if (is_expr && res.value.type != VAL_NULL) {
                value_print_repr(res.value);
                printf("\n");
            }
        }

        value_release(res.value);
        ast_free(ast);
        parser_free(&parser);
    }

    env_release(globals);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        run_repl();
        return 0;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            print_version();
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_help();
            return 0;
        }
        return run_file(argv[1]);
    }

    if (argc == 3) {
        if (strcmp(argv[1], "--tokens") == 0) {
            return run_tokens(argv[2]);
        }
        if (strcmp(argv[1], "--ast") == 0) {
            return run_ast(argv[2]);
        }
    }

    fprintf(stderr, "Error: invalid arguments. Use 'mrt --help' for usage.\n");
    return 1;
}

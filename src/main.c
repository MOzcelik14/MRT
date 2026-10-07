#include "common.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "interpreter.h"
#include "builtin.h"
#include "error.h"
#include "formatter.h"
#include "inspector.h"

static void print_version(void) {
    printf("%s %s\n", MRT_NAME, MRT_VERSION);
}

static void print_help(void) {
    printf("MRT Programming Language (%s)\n\n", MRT_VERSION);
    printf("Usage:\n");
    printf("  mrt [command] [options] [file.mrt]\n\n");
    printf("Commands:\n");
    printf("  run <file.mrt>               Execute an MRT script (default if file given)\n");
    printf("  check [options] <file.mrt>   Check syntax without executing\n");
    printf("  fmt [options] <file.mrt>     Format source code\n");
    printf("  inspect [options] <file.mrt> Inspect AST symbols and structure\n");
    printf("  repl                         Start interactive REPL session\n");
    printf("  version                      Display version information\n");
    printf("  help                         Display this help message\n\n");
    printf("Options:\n");
    printf("  --diagnostics=json           Output syntax errors as JSON (with 'check')\n");
    printf("  --check                      Check formatting without modifying file (with 'fmt')\n");
    printf("  --symbols                    Inspect declared symbols (with 'inspect')\n");
    printf("  --json                       Output symbols as JSON (with 'inspect')\n");
    printf("  --tokens                     Tokenize file and print token stream\n");
    printf("  --ast                        Parse file and print abstract syntax tree\n");
    printf("  -v, --version                Display version information\n");
    printf("  -h, --help                   Display this help message\n\n");
    printf("Examples:\n");
    printf("  mrt main.mrt\n");
    printf("  mrt run main.mrt\n");
    printf("  mrt check --diagnostics=json main.mrt\n");
    printf("  mrt fmt --check main.mrt\n");
    printf("  mrt inspect --symbols --json main.mrt\n\n");
    printf("If no file or command is provided, MRT starts in interactive REPL mode.\n");
}

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 0) {
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
    if (!source) {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        return 1;
    }

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
    if (!source) {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        return 1;
    }

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

static int run_check(const char *filename, bool json_mode) {
    mrt_diagnostics_init();
    mrt_diagnostics_set_json_mode(json_mode);

    char *source = read_file(filename);
    if (!source) {
        if (json_mode) {
            mrt_diagnostics_add(filename, 1, 1, "error", "IOError", "could not open or read file");
            mrt_diagnostics_print_json(stdout);
            mrt_diagnostics_free();
        } else {
            fprintf(stderr, "Error: could not open file '%s'\n", filename);
        }
        return 1;
    }

    Lexer lexer;
    lexer_init(&lexer, source, filename);
    TokenArray tokens = lexer_tokenize_all(&lexer);

    Parser parser;
    parser_init(&parser, tokens, source, filename);
    ASTNode *ast = parser_parse(&parser);

    int exit_code = 0;
    if (json_mode) {
        mrt_diagnostics_print_json(stdout);
        if (mrt_diagnostics_count() > 0 || parser.had_error) {
            exit_code = 1;
        }
    } else {
        if (parser.had_error || !ast) {
            exit_code = 1;
        } else {
            printf("Syntax OK: %s\n", filename);
        }
    }

    if (ast) ast_free(ast);
    parser_free(&parser);
    mrt_free(source);
    mrt_diagnostics_free();
    return exit_code;
}

static int run_fmt(const char *filename, bool check_only) {
    bool differs = false;
    bool ok = mrt_format_file(filename, check_only, &differs);
    if (!ok) {
        fprintf(stderr, "Error: failed to format '%s'\n", filename);
        return 1;
    }

    if (check_only) {
        if (differs) {
            fprintf(stderr, "%s: formatting needed\n", filename);
            return 1;
        }
        printf("%s: already formatted\n", filename);
        return 0;
    }

    if (differs) {
        printf("Formatted: %s\n", filename);
    }
    return 0;
}

static int run_inspect(const char *filename, bool json_mode) {
    char *source = read_file(filename);
    if (!source) {
        if (json_mode) {
            printf("{\n  \"file\": \"%s\",\n  \"error\": \"could not open file\",\n  \"symbols\": []\n}\n", filename);
        } else {
            fprintf(stderr, "Error: could not open file '%s'\n", filename);
        }
        return 1;
    }

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

    MrtSymbolList symbols;
    mrt_symbol_list_init(&symbols);
    mrt_inspect_ast(ast, &symbols);

    if (json_mode) {
        mrt_print_symbols_json(filename, &symbols);
    } else {
        mrt_print_symbols_human(filename, &symbols);
    }

    mrt_symbol_list_free(&symbols);
    ast_free(ast);
    parser_free(&parser);
    mrt_free(source);
    return 0;
}

static int run_file(const char *filename) {
    char *source = read_file(filename);
    if (!source) {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        return 1;
    }

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

    const char *first = argv[1];

    if (strcmp(first, "--version") == 0 || strcmp(first, "-v") == 0 || strcmp(first, "version") == 0) {
        print_version();
        return 0;
    }

    if (strcmp(first, "--help") == 0 || strcmp(first, "-h") == 0 || strcmp(first, "help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(first, "repl") == 0) {
        run_repl();
        return 0;
    }

    if (strcmp(first, "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: 'mrt run' requires a file path.\n");
            return 1;
        }
        return run_file(argv[2]);
    }

    if (strcmp(first, "check") == 0) {
        bool json_mode = false;
        const char *filename = NULL;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--diagnostics=json") == 0) {
                json_mode = true;
            } else if (argv[i][0] != '-') {
                filename = argv[i];
            } else {
                fprintf(stderr, "Error: unknown option '%s' for 'mrt check'\n", argv[i]);
                return 1;
            }
        }
        if (!filename) {
            fprintf(stderr, "Error: 'mrt check' requires a file path.\n");
            return 1;
        }
        return run_check(filename, json_mode);
    }

    if (strcmp(first, "fmt") == 0) {
        bool check_only = false;
        const char *filename = NULL;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--check") == 0) {
                check_only = true;
            } else if (argv[i][0] != '-') {
                filename = argv[i];
            } else {
                fprintf(stderr, "Error: unknown option '%s' for 'mrt fmt'\n", argv[i]);
                return 1;
            }
        }
        if (!filename) {
            fprintf(stderr, "Error: 'mrt fmt' requires a file path.\n");
            return 1;
        }
        return run_fmt(filename, check_only);
    }

    if (strcmp(first, "inspect") == 0) {
        bool json_mode = false;
        bool symbols = false;
        const char *filename = NULL;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--json") == 0) {
                json_mode = true;
            } else if (strcmp(argv[i], "--symbols") == 0) {
                symbols = true;
            } else if (argv[i][0] != '-') {
                filename = argv[i];
            } else {
                fprintf(stderr, "Error: unknown option '%s' for 'mrt inspect'\n", argv[i]);
                return 1;
            }
        }
        (void)symbols;
        if (!filename) {
            fprintf(stderr, "Error: 'mrt inspect' requires a file path.\n");
            return 1;
        }
        return run_inspect(filename, json_mode);
    }

    if (strcmp(first, "--tokens") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: '--tokens' requires a file path.\n");
            return 1;
        }
        return run_tokens(argv[2]);
    }

    if (strcmp(first, "--ast") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: '--ast' requires a file path.\n");
            return 1;
        }
        return run_ast(argv[2]);
    }

    if (first[0] == '-') {
        fprintf(stderr, "Error: unknown option '%s'. Use 'mrt --help' for usage.\n", first);
        return 1;
    }

    /* Default: mrt <file.mrt> */
    return run_file(first);
}

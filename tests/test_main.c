#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "../src/common.h"
#include "../src/token.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/ast.h"
#include "../src/value.h"
#include "../src/environment.h"
#include "../src/interpreter.h"
#include "../src/builtin.h"

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(name) \
    do { \
        printf("[RUN] %-35s ", #name); \
        tests_run++; \
        name(); \
        tests_passed++; \
        printf("PASSED\n"); \
    } while (0)

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "\nFAILED: %s (line %d)\n", #expr, __LINE__); \
            exit(1); \
        } \
    } while (0)

static EvalResult run_code(const char *source) {
    Lexer lexer;
    lexer_init(&lexer, source, "test.mrt");
    TokenArray tokens = lexer_tokenize_all(&lexer);

    if (lexer.had_error) {
        token_array_free(&tokens);
        EvalResult res;
        res.status = INTERP_ERROR;
        res.value = value_none();
        return res;
    }

    Parser parser;
    parser_init(&parser, tokens, source, "test.mrt");
    ASTNode *ast = parser_parse(&parser);

    if (!ast) {
        parser_free(&parser);
        EvalResult res;
        res.status = INTERP_ERROR;
        res.value = value_none();
        return res;
    }

    Interpreter interp;
    interpreter_init(&interp, source, "test.mrt");
    EvalResult res = interpreter_interpret(&interp, ast);

    interpreter_free(&interp);
    ast_free(ast);
    parser_free(&parser);
    return res;
}

static void test_lexer_tokens(void) {
    const char *code = "var x = 123 + 4.56 // comment\ngive \"abc\"";
    Lexer lexer;
    lexer_init(&lexer, code, "test.mrt");
    TokenArray tokens = lexer_tokenize_all(&lexer);

    ASSERT(tokens.count > 0);
    ASSERT(tokens.tokens[0].type == TOKEN_VAR);
    ASSERT(tokens.tokens[1].type == TOKEN_IDENTIFIER);
    ASSERT(strcmp(tokens.tokens[1].lexeme, "x") == 0);
    ASSERT(tokens.tokens[2].type == TOKEN_EQUAL);
    ASSERT(tokens.tokens[3].type == TOKEN_INT);
    ASSERT(tokens.tokens[3].as.int_val == 123);
    ASSERT(tokens.tokens[4].type == TOKEN_PLUS);
    ASSERT(tokens.tokens[5].type == TOKEN_FLOAT);
    ASSERT(tokens.tokens[7].type == TOKEN_GIVE);
    ASSERT(tokens.tokens[8].type == TOKEN_STRING);
    ASSERT(strcmp(tokens.tokens[8].as.string_val, "abc") == 0);

    token_array_free(&tokens);

    // Test block comments
    const char *code2 = "/* block comment */ var y = yes";
    Lexer lexer2;
    lexer_init(&lexer2, code2, "test.mrt");
    TokenArray tokens2 = lexer_tokenize_all(&lexer2);
    ASSERT(tokens2.tokens[0].type == TOKEN_VAR);
    ASSERT(tokens2.tokens[3].type == TOKEN_YES);
    token_array_free(&tokens2);
}

static void test_numbers(void) {
    EvalResult r1 = run_code("42");
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_INT);
    ASSERT(r1.value.as.int_val == 42);
    value_release(r1.value);

    EvalResult r2 = run_code("3.14");
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_FLOAT);
    ASSERT(r2.value.as.float_val > 3.13 && r2.value.as.float_val < 3.15);
    value_release(r2.value);
}

static void test_strings(void) {
    EvalResult r1 = run_code("\"Merhaba\"");
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_STRING);
    ASSERT(strcmp(r1.value.as.string_val->chars, "Merhaba") == 0);
    value_release(r1.value);

    EvalResult r2 = run_code("\"satir1\\nsatir2\\t\\\"tab\\\"\"");
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_STRING);
    ASSERT(strcmp(r2.value.as.string_val->chars, "satir1\nsatir2\t\"tab\"") == 0);
    value_release(r2.value);

    EvalResult r3 = run_code("\"Hello \" + \"World\"");
    ASSERT(r3.status == INTERP_OK);
    ASSERT(r3.value.type == VAL_STRING);
    ASSERT(strcmp(r3.value.as.string_val->chars, "Hello World") == 0);
    value_release(r3.value);
}

static void test_operators_and_precedence(void) {
    // 10 + 5 * 2 == 20
    EvalResult r1 = run_code("10 + 5 * 2");
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_INT);
    ASSERT(r1.value.as.int_val == 20);
    value_release(r1.value);

    // (10 + 5) * 2 == 30
    EvalResult r2 = run_code("(10 + 5) * 2");
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_INT);
    ASSERT(r2.value.as.int_val == 30);
    value_release(r2.value);

    // 1 + 2 * 3 == 7
    EvalResult r3 = run_code("1 + 2 * 3 == 7");
    ASSERT(r3.status == INTERP_OK);
    ASSERT(r3.value.type == VAL_BOOL);
    ASSERT(r3.value.as.bool_val == true);
    value_release(r3.value);

    // Left associativity of -: 10 - 4 - 2 == 4
    EvalResult r4 = run_code("10 - 4 - 2");
    ASSERT(r4.status == INTERP_OK);
    ASSERT(r4.value.type == VAL_INT);
    ASSERT(r4.value.as.int_val == 4);
    value_release(r4.value);

    // Modulo
    EvalResult r5 = run_code("14 % 4");
    ASSERT(r5.status == INTERP_OK);
    ASSERT(r5.value.type == VAL_INT);
    ASSERT(r5.value.as.int_val == 2);
    value_release(r5.value);

    // Comparisons
    EvalResult r6 = run_code("5 > 3 and 10 <= 10 and 4 != 5 and 6 == 6");
    ASSERT(r6.status == INTERP_OK);
    ASSERT(r6.value.type == VAL_BOOL);
    ASSERT(r6.value.as.bool_val == true);
    value_release(r6.value);
}

static void test_variables_and_assignment(void) {
    EvalResult r1 = run_code("var x = 10\nx = 25\nx");
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_INT);
    ASSERT(r1.value.as.int_val == 25);
    value_release(r1.value);

    EvalResult r2 = run_code("var a = 15; var b = 30; a + b");
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_INT);
    ASSERT(r2.value.as.int_val == 45);
    value_release(r2.value);
}

static void test_boolean_logic(void) {
    EvalResult r1 = run_code("not no");
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_BOOL);
    ASSERT(r1.value.as.bool_val == true);
    value_release(r1.value);

    EvalResult r2 = run_code("not yes");
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_BOOL);
    ASSERT(r2.value.as.bool_val == false);
    value_release(r2.value);

    // Short-circuit: no and division by zero should NOT error
    EvalResult r3 = run_code("no and (1 / 0 == 0)");
    ASSERT(r3.status == INTERP_OK);
    ASSERT(r3.value.type == VAL_BOOL);
    ASSERT(r3.value.as.bool_val == false);
    value_release(r3.value);

    // Short-circuit: yes or division by zero should NOT error
    EvalResult r4 = run_code("yes or (1 / 0 == 0)");
    ASSERT(r4.status == INTERP_OK);
    ASSERT(r4.value.type == VAL_BOOL);
    ASSERT(r4.value.as.bool_val == true);
    value_release(r4.value);
}

static void test_scope_and_shadowing(void) {
    const char *code =
        "var x = 10\n"
        "{\n"
        "    var x = 20\n"
        "}\n"
        "x\n";
    EvalResult r1 = run_code(code);
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_INT);
    ASSERT(r1.value.as.int_val == 10);
    value_release(r1.value);

    const char *code2 =
        "var x = 10\n"
        "{\n"
        "    x = 30\n"
        "}\n"
        "x\n";
    EvalResult r2 = run_code(code2);
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_INT);
    ASSERT(r2.value.as.int_val == 30);
    value_release(r2.value);
}

static void test_functions_and_returns(void) {
    const char *code =
        "task topla(a, b) {\n"
        "    give a + b\n"
        "}\n"
        "topla(100, 250)\n";
    EvalResult r = run_code(code);
    ASSERT(r.status == INTERP_OK);
    ASSERT(r.value.type == VAL_INT);
    ASSERT(r.value.as.int_val == 350);
    value_release(r.value);

    const char *code_nested =
        "task kare(x) { give x * x }\n"
        "task hipotenus_kare(a, b) { give kare(a) + kare(b) }\n"
        "hipotenus_kare(3, 4)\n";
    EvalResult r2 = run_code(code_nested);
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_INT);
    ASSERT(r2.value.as.int_val == 25);
    value_release(r2.value);
}

static void test_recursion(void) {
    const char *fac_code =
        "task factorial(n) {\n"
        "    when n <= 1 {\n"
        "        give 1\n"
        "    }\n"
        "    give n * factorial(n - 1)\n"
        "}\n"
        "factorial(6)\n";
    EvalResult r = run_code(fac_code);
    ASSERT(r.status == INTERP_OK);
    ASSERT(r.value.type == VAL_INT);
    ASSERT(r.value.as.int_val == 720);
    value_release(r.value);

    const char *fib_code =
        "task fib(n) {\n"
        "    when n <= 1 {\n"
        "        give n\n"
        "    }\n"
        "    give fib(n - 1) + fib(n - 2)\n"
        "}\n"
        "fib(8)\n";
    EvalResult r_fib = run_code(fib_code);
    ASSERT(r_fib.status == INTERP_OK);
    ASSERT(r_fib.value.type == VAL_INT);
    ASSERT(r_fib.value.as.int_val == 21);
    value_release(r_fib.value);
}

static void test_control_flow(void) {
    const char *if_code =
        "var x = 10\n"
        "var res = \"\"\n"
        "when x > 5 {\n"
        "    res = \"buyuk\"\n"
        "} otherwise {\n"
        "    res = \"kucuk\"\n"
        "}\n"
        "res\n";
    EvalResult r1 = run_code(if_code);
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_STRING);
    ASSERT(strcmp(r1.value.as.string_val->chars, "buyuk") == 0);
    value_release(r1.value);

    const char *while_code =
        "var i = 0\n"
        "var sum = 0\n"
        "repeat i < 10 {\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "}\n"
        "sum\n";
    EvalResult r2 = run_code(while_code);
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_INT);
    ASSERT(r2.value.as.int_val == 45);
    value_release(r2.value);
}

static void test_break_and_continue(void) {
    const char *break_code =
        "var i = 0\n"
        "var sum = 0\n"
        "repeat i < 10 {\n"
        "    i = i + 1\n"
        "    when i == 5 {\n"
        "        break\n"
        "    }\n"
        "    sum = sum + i\n"
        "}\n"
        "sum\n";
    EvalResult r1 = run_code(break_code);
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_INT);
    ASSERT(r1.value.as.int_val == 10); // 1 + 2 + 3 + 4 = 10
    value_release(r1.value);

    const char *cont_code =
        "var i = 0\n"
        "var sum = 0\n"
        "repeat i < 6 {\n"
        "    i = i + 1\n"
        "    when i == 3 {\n"
        "        continue\n"
        "    }\n"
        "    sum = sum + i\n"
        "}\n"
        "sum\n";
    EvalResult r2 = run_code(cont_code);
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_INT);
    ASSERT(r2.value.as.int_val == 18); // 1 + 2 + 4 + 5 + 6 = 18
    value_release(r2.value);
}

static void test_say_statement(void) {
    const char *code =
        "var x = 100\n"
        "say \"Saying: \" + toText(x)\n";
    EvalResult r = run_code(code);
    ASSERT(r.status == INTERP_OK);
    value_release(r.value);
}

static void test_builtins(void) {
    EvalResult r1 = run_code("typeOf(10)");
    ASSERT(r1.status == INTERP_OK);
    ASSERT(r1.value.type == VAL_STRING);
    ASSERT(strcmp(r1.value.as.string_val->chars, "integer") == 0);
    value_release(r1.value);

    EvalResult r2 = run_code("typeOf(\"Murat\")");
    ASSERT(r2.status == INTERP_OK);
    ASSERT(r2.value.type == VAL_STRING);
    ASSERT(strcmp(r2.value.as.string_val->chars, "string") == 0);
    value_release(r2.value);

    EvalResult r_none = run_code("typeOf(none)");
    ASSERT(r_none.status == INTERP_OK);
    ASSERT(r_none.value.type == VAL_STRING);
    ASSERT(strcmp(r_none.value.as.string_val->chars, "none") == 0);
    value_release(r_none.value);

    EvalResult r_bool = run_code("typeOf(yes)");
    ASSERT(r_bool.status == INTERP_OK);
    ASSERT(r_bool.value.type == VAL_STRING);
    ASSERT(strcmp(r_bool.value.as.string_val->chars, "boolean") == 0);
    value_release(r_bool.value);

    EvalResult r3 = run_code("length(\"Murat\")");
    ASSERT(r3.status == INTERP_OK);
    ASSERT(r3.value.type == VAL_INT);
    ASSERT(r3.value.as.int_val == 5);
    value_release(r3.value);

    EvalResult r4 = run_code("toText(456)");
    ASSERT(r4.status == INTERP_OK);
    ASSERT(r4.value.type == VAL_STRING);
    ASSERT(strcmp(r4.value.as.string_val->chars, "456") == 0);
    value_release(r4.value);

    // toNumber tests
    EvalResult r5 = run_code("toNumber(\"123\")");
    ASSERT(r5.status == INTERP_OK);
    ASSERT(r5.value.type == VAL_INT);
    ASSERT(r5.value.as.int_val == 123);
    value_release(r5.value);

    EvalResult r6 = run_code("toNumber(\"-45\")");
    ASSERT(r6.status == INTERP_OK);
    ASSERT(r6.value.type == VAL_INT);
    ASSERT(r6.value.as.int_val == -45);
    value_release(r6.value);

    EvalResult r7 = run_code("toNumber(\"3.14\")");
    ASSERT(r7.status == INTERP_OK);
    ASSERT(r7.value.type == VAL_FLOAT);
    ASSERT(r7.value.as.float_val > 3.13 && r7.value.as.float_val < 3.15);
    value_release(r7.value);

    EvalResult r8 = run_code("toNumber(99)");
    ASSERT(r8.status == INTERP_OK);
    ASSERT(r8.value.type == VAL_INT);
    ASSERT(r8.value.as.int_val == 99);
    value_release(r8.value);

    EvalResult r9 = run_code("toNumber(yes)");
    ASSERT(r9.status == INTERP_OK);
    ASSERT(r9.value.type == VAL_INT);
    ASSERT(r9.value.as.int_val == 1);
    value_release(r9.value);

    // read and toNumber function existence
    EvalResult r_fn1 = run_code("typeOf(read)");
    ASSERT(r_fn1.status == INTERP_OK);
    ASSERT(r_fn1.value.type == VAL_STRING);
    ASSERT(strcmp(r_fn1.value.as.string_val->chars, "function") == 0);
    value_release(r_fn1.value);

    EvalResult r_fn2 = run_code("typeOf(toNumber)");
    ASSERT(r_fn2.status == INTERP_OK);
    ASSERT(r_fn2.value.type == VAL_STRING);
    ASSERT(strcmp(r_fn2.value.as.string_val->chars, "function") == 0);
    value_release(r_fn2.value);
}

static void test_errors(void) {
    // Syntax error
    EvalResult r_syntax = run_code("var = 10");
    ASSERT(r_syntax.status == INTERP_ERROR);

    // Name error
    EvalResult r_name = run_code("say tanimsiz_degisken");
    ASSERT(r_name.status == INTERP_ERROR);

    // Type error
    EvalResult r_type = run_code("10 + yes");
    ASSERT(r_type.status == INTERP_ERROR);

    // Type error with length
    EvalResult r_len = run_code("length(100)");
    ASSERT(r_len.status == INTERP_ERROR);

    // Type error with toNumber invalid string
    EvalResult r_num1 = run_code("toNumber(\"abc\")");
    ASSERT(r_num1.status == INTERP_ERROR);

    EvalResult r_num2 = run_code("toNumber(\"\")");
    ASSERT(r_num2.status == INTERP_ERROR);

    EvalResult r_read_err = run_code("read(\"a\", \"b\")");
    ASSERT(r_read_err.status == INTERP_ERROR);

    // Division by zero
    EvalResult r_div = run_code("10 / 0");
    ASSERT(r_div.status == INTERP_ERROR);

    EvalResult r_mod = run_code("10 % 0");
    ASSERT(r_mod.status == INTERP_ERROR);
}

int main(void) {
    printf("=========================================\n");
    printf("   MRT Programming Language Test Suite   \n");
    printf("=========================================\n");

    RUN_TEST(test_lexer_tokens);
    RUN_TEST(test_numbers);
    RUN_TEST(test_strings);
    RUN_TEST(test_operators_and_precedence);
    RUN_TEST(test_variables_and_assignment);
    RUN_TEST(test_boolean_logic);
    RUN_TEST(test_scope_and_shadowing);
    RUN_TEST(test_functions_and_returns);
    RUN_TEST(test_recursion);
    RUN_TEST(test_control_flow);
    RUN_TEST(test_break_and_continue);
    RUN_TEST(test_say_statement);
    RUN_TEST(test_builtins);
    RUN_TEST(test_errors);

    printf("=========================================\n");
    printf("All %d tests passed successfully!\n", tests_passed);
    printf("=========================================\n");
    return 0;
}

#include "inspector.h"

void mrt_symbol_list_init(MrtSymbolList *list) {
    list->count = 0;
    list->capacity = 8;
    list->symbols = (MrtSymbol *)mrt_malloc(sizeof(MrtSymbol) * list->capacity);
}

void mrt_symbol_list_free(MrtSymbolList *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        mrt_free(list->symbols[i].name);
        for (size_t j = 0; j < list->symbols[i].param_count; j++) {
            mrt_free(list->symbols[i].params[j]);
        }
        mrt_free(list->symbols[i].params);
    }
    mrt_free(list->symbols);
    list->symbols = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void add_symbol(MrtSymbolList *list, const char *name, const char *kind,
                       int line, int col, char **params, size_t param_count) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity * 2;
        list->symbols = (MrtSymbol *)mrt_realloc(list->symbols, sizeof(MrtSymbol) * list->capacity);
    }
    MrtSymbol *sym = &list->symbols[list->count++];
    sym->name = mrt_strdup(name);
    sym->kind = kind;
    sym->line = line;
    sym->column = col;
    sym->param_count = param_count;
    if (param_count > 0 && params) {
        sym->params = (char **)mrt_malloc(sizeof(char *) * param_count);
        for (size_t i = 0; i < param_count; i++) {
            sym->params[i] = mrt_strdup(params[i]);
        }
    } else {
        sym->params = NULL;
    }
}

void mrt_inspect_ast(const ASTNode *node, MrtSymbolList *list) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            for (size_t i = 0; i < node->as.block.count; i++) {
                mrt_inspect_ast(node->as.block.statements[i], list);
            }
            break;

        case AST_FUNCTION_DECL:
            add_symbol(list, node->as.func_decl.name, "task",
                       node->line, node->column,
                       node->as.func_decl.params, node->as.func_decl.param_count);
            mrt_inspect_ast(node->as.func_decl.body, list);
            break;

        case AST_VAR_DECL:
            add_symbol(list, node->as.var_decl.name, "variable",
                       node->line, node->column, NULL, 0);
            if (node->as.var_decl.init) {
                mrt_inspect_ast(node->as.var_decl.init, list);
            }
            break;

        case AST_IF:
            mrt_inspect_ast(node->as.if_stmt.then_branch, list);
            if (node->as.if_stmt.else_branch) {
                mrt_inspect_ast(node->as.if_stmt.else_branch, list);
            }
            break;

        case AST_WHILE:
            mrt_inspect_ast(node->as.while_stmt.body, list);
            break;

        case AST_EACH:
            add_symbol(list, node->as.each_stmt.var_name, "variable",
                       node->line, node->column, NULL, 0);
            mrt_inspect_ast(node->as.each_stmt.body, list);
            break;

        default:
            break;
    }
}

void mrt_print_symbols_human(const char *filename, const MrtSymbolList *list) {
    printf("Symbols in %s:\n", filename ? filename : "stdin");
    printf("  Tasks:\n");
    bool has_tasks = false;
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->symbols[i].kind, "task") == 0) {
            has_tasks = true;
            printf("    - %s(", list->symbols[i].name);
            for (size_t p = 0; p < list->symbols[i].param_count; p++) {
                printf("%s%s", list->symbols[i].params[p],
                       p + 1 < list->symbols[i].param_count ? ", " : "");
            }
            printf(") at line %d\n", list->symbols[i].line);
        }
    }
    if (!has_tasks) printf("    (none)\n");

    printf("  Variables:\n");
    bool has_vars = false;
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->symbols[i].kind, "variable") == 0) {
            has_vars = true;
            printf("    - %s at line %d\n", list->symbols[i].name, list->symbols[i].line);
        }
    }
    if (!has_vars) printf("    (none)\n");
}

void mrt_print_symbols_json(const char *filename, const MrtSymbolList *list) {
    printf("{\n");
    printf("  \"file\": \"%s\",\n", filename ? filename : "stdin");
    printf("  \"symbols\": [\n");
    for (size_t i = 0; i < list->count; i++) {
        MrtSymbol *s = &list->symbols[i];
        printf("    {\n");
        printf("      \"name\": \"%s\",\n", s->name);
        printf("      \"kind\": \"%s\",\n", s->kind);
        printf("      \"line\": %d,\n", s->line);
        printf("      \"column\": %d", s->column);
        if (strcmp(s->kind, "task") == 0) {
            printf(",\n      \"params\": [");
            for (size_t p = 0; p < s->param_count; p++) {
                printf("\"%s\"%s", s->params[p], p + 1 < s->param_count ? ", " : "");
            }
            printf("]\n");
        } else {
            printf("\n");
        }
        printf("    }%s\n", i + 1 < list->count ? "," : "");
    }
    printf("  ]\n");
    printf("}\n");
}

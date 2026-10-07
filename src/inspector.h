#ifndef MRT_INSPECTOR_H
#define MRT_INSPECTOR_H

#include "common.h"
#include "ast.h"

typedef struct {
    char *name;
    const char *kind; /* "task" or "variable" */
    int line;
    int column;
    char **params;
    size_t param_count;
} MrtSymbol;

typedef struct {
    MrtSymbol *symbols;
    size_t count;
    size_t capacity;
} MrtSymbolList;

void mrt_symbol_list_init(MrtSymbolList *list);
void mrt_symbol_list_free(MrtSymbolList *list);
void mrt_inspect_ast(const ASTNode *node, MrtSymbolList *list);

void mrt_print_symbols_human(const char *filename, const MrtSymbolList *list);
void mrt_print_symbols_json(const char *filename, const MrtSymbolList *list);

#endif /* MRT_INSPECTOR_H */

#ifndef MRT_FORMATTER_H
#define MRT_FORMATTER_H

#include "common.h"
#include "ast.h"

/* Formats an ASTNode program into formatted MRT source code string */
char *mrt_format_ast(const ASTNode *node);

/* Formats a source file in place. Returns true if successful. */
bool mrt_format_file(const char *filename, bool check_only, bool *out_differs);

#endif /* MRT_FORMATTER_H */

#include "ast.h"

static ASTNode *ast_alloc_node(ASTNodeType type, int line, int col) {
    ASTNode *node = (ASTNode *)mrt_malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->line = line;
    node->column = col;
    return node;
}

ASTNode *ast_new_program(int line, int col) {
    ASTNode *node = ast_alloc_node(AST_PROGRAM, line, col);
    node->as.block.statements = NULL;
    node->as.block.count = 0;
    node->as.block.capacity = 0;
    return node;
}

void ast_program_add_stmt(ASTNode *prog, ASTNode *stmt) {
    if (!prog || !stmt) return;
    if (prog->as.block.count + 1 > prog->as.block.capacity) {
        size_t new_cap = prog->as.block.capacity < 8 ? 8 : prog->as.block.capacity * 2;
        prog->as.block.statements = (ASTNode **)mrt_realloc(
            prog->as.block.statements, new_cap * sizeof(ASTNode *));
        prog->as.block.capacity = new_cap;
    }
    prog->as.block.statements[prog->as.block.count++] = stmt;
}

ASTNode *ast_new_block(int line, int col) {
    ASTNode *node = ast_alloc_node(AST_BLOCK, line, col);
    node->as.block.statements = NULL;
    node->as.block.count = 0;
    node->as.block.capacity = 0;
    return node;
}

void ast_block_add_stmt(ASTNode *block, ASTNode *stmt) {
    ast_program_add_stmt(block, stmt);
}

ASTNode *ast_new_var_decl(char *name, ASTNode *init, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_VAR_DECL, line, col);
    node->as.var_decl.name = name;
    node->as.var_decl.init = init;
    return node;
}

ASTNode *ast_new_assign(char *name, ASTNode *value, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_ASSIGN, line, col);
    node->as.assign.name = name;
    node->as.assign.value = value;
    return node;
}

ASTNode *ast_new_literal_int(int64_t val, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_LITERAL, line, col);
    node->as.literal.lit_type = LITERAL_INT;
    node->as.literal.as.int_val = val;
    return node;
}

ASTNode *ast_new_literal_float(double val, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_LITERAL, line, col);
    node->as.literal.lit_type = LITERAL_FLOAT;
    node->as.literal.as.float_val = val;
    return node;
}

ASTNode *ast_new_literal_string(char *val, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_LITERAL, line, col);
    node->as.literal.lit_type = LITERAL_STRING;
    node->as.literal.as.string_val = val;
    return node;
}

ASTNode *ast_new_literal_bool(bool val, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_LITERAL, line, col);
    node->as.literal.lit_type = LITERAL_BOOL;
    node->as.literal.as.bool_val = val;
    return node;
}

ASTNode *ast_new_literal_none(int line, int col) {
    ASTNode *node = ast_alloc_node(AST_LITERAL, line, col);
    node->as.literal.lit_type = LITERAL_NONE;
    return node;
}

ASTNode *ast_new_literal_null(int line, int col) {
    return ast_new_literal_none(line, col);
}

ASTNode *ast_new_array_literal(ASTNode **elements, size_t count, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_ARRAY_LITERAL, line, col);
    node->as.array_literal.elements = elements;
    node->as.array_literal.count = count;
    return node;
}

ASTNode *ast_new_map_literal(char **keys, ASTNode **values, size_t count, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_MAP_LITERAL, line, col);
    node->as.map_literal.keys = keys;
    node->as.map_literal.values = values;
    node->as.map_literal.count = count;
    return node;
}

ASTNode *ast_new_identifier(char *name, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_IDENTIFIER, line, col);
    node->as.identifier.name = name;
    return node;
}

ASTNode *ast_new_binary(TokenType op, ASTNode *left, ASTNode *right, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_BINARY, line, col);
    node->as.binary.op = op;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

ASTNode *ast_new_unary(TokenType op, ASTNode *operand, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_UNARY, line, col);
    node->as.unary.op = op;
    node->as.unary.operand = operand;
    return node;
}

ASTNode *ast_new_func_decl(char *name, char **params, size_t param_count, ASTNode *body, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_FUNCTION_DECL, line, col);
    node->as.func_decl.name = name;
    node->as.func_decl.params = params;
    node->as.func_decl.param_count = param_count;
    node->as.func_decl.body = body;
    return node;
}

ASTNode *ast_new_func_call(ASTNode *callee, ASTNode **args, size_t arg_count, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_FUNCTION_CALL, line, col);
    node->as.func_call.callee = callee;
    node->as.func_call.args = args;
    node->as.func_call.arg_count = arg_count;
    return node;
}

ASTNode *ast_new_index_get(ASTNode *target, ASTNode *index, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_INDEX_GET, line, col);
    node->as.index_get.target = target;
    node->as.index_get.index = index;
    return node;
}

ASTNode *ast_new_index_set(ASTNode *target, ASTNode *index, ASTNode *value, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_INDEX_SET, line, col);
    node->as.index_set.target = target;
    node->as.index_set.index = index;
    node->as.index_set.value = value;
    return node;
}

ASTNode *ast_new_if(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_IF, line, col);
    node->as.if_stmt.condition = condition;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode *ast_new_while(ASTNode *condition, ASTNode *body, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_WHILE, line, col);
    node->as.while_stmt.condition = condition;
    node->as.while_stmt.body = body;
    return node;
}

ASTNode *ast_new_each(char *var_name, ASTNode *collection, ASTNode *body, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_EACH, line, col);
    node->as.each_stmt.var_name = var_name;
    node->as.each_stmt.collection = collection;
    node->as.each_stmt.body = body;
    return node;
}

ASTNode *ast_new_return(ASTNode *value, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_RETURN, line, col);
    node->as.return_stmt.value = value;
    return node;
}

ASTNode *ast_new_say(ASTNode *value, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_SAY, line, col);
    node->as.say_stmt.value = value;
    return node;
}

ASTNode *ast_new_break(int line, int col) {
    return ast_alloc_node(AST_BREAK, line, col);
}

ASTNode *ast_new_continue(int line, int col) {
    return ast_alloc_node(AST_CONTINUE, line, col);
}

ASTNode *ast_new_use(char *path, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_USE, line, col);
    node->as.use_stmt.path = path;
    return node;
}

ASTNode *ast_new_expr_stmt(ASTNode *expr, int line, int col) {
    ASTNode *node = ast_alloc_node(AST_EXPR_STMT, line, col);
    node->as.expr_stmt.expression = expr;
    return node;
}

void ast_free(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            for (size_t i = 0; i < node->as.block.count; i++) {
                ast_free(node->as.block.statements[i]);
            }
            mrt_free(node->as.block.statements);
            break;

        case AST_VAR_DECL:
            mrt_free(node->as.var_decl.name);
            ast_free(node->as.var_decl.init);
            break;

        case AST_ASSIGN:
            mrt_free(node->as.assign.name);
            ast_free(node->as.assign.value);
            break;

        case AST_LITERAL:
            if (node->as.literal.lit_type == LITERAL_STRING) {
                mrt_free(node->as.literal.as.string_val);
            }
            break;

        case AST_ARRAY_LITERAL:
            for (size_t i = 0; i < node->as.array_literal.count; i++) {
                ast_free(node->as.array_literal.elements[i]);
            }
            mrt_free(node->as.array_literal.elements);
            break;

        case AST_MAP_LITERAL:
            for (size_t i = 0; i < node->as.map_literal.count; i++) {
                mrt_free(node->as.map_literal.keys[i]);
                ast_free(node->as.map_literal.values[i]);
            }
            mrt_free(node->as.map_literal.keys);
            mrt_free(node->as.map_literal.values);
            break;

        case AST_INDEX_GET:
            ast_free(node->as.index_get.target);
            ast_free(node->as.index_get.index);
            break;

        case AST_INDEX_SET:
            ast_free(node->as.index_set.target);
            ast_free(node->as.index_set.index);
            ast_free(node->as.index_set.value);
            break;

        case AST_EACH:
            mrt_free(node->as.each_stmt.var_name);
            ast_free(node->as.each_stmt.collection);
            ast_free(node->as.each_stmt.body);
            break;

        case AST_USE:
            mrt_free(node->as.use_stmt.path);
            break;

        case AST_IDENTIFIER:
            mrt_free(node->as.identifier.name);
            break;

        case AST_BINARY:
            ast_free(node->as.binary.left);
            ast_free(node->as.binary.right);
            break;

        case AST_UNARY:
            ast_free(node->as.unary.operand);
            break;

        case AST_FUNCTION_DECL:
            mrt_free(node->as.func_decl.name);
            for (size_t i = 0; i < node->as.func_decl.param_count; i++) {
                mrt_free(node->as.func_decl.params[i]);
            }
            mrt_free(node->as.func_decl.params);
            ast_free(node->as.func_decl.body);
            break;

        case AST_FUNCTION_CALL:
            ast_free(node->as.func_call.callee);
            for (size_t i = 0; i < node->as.func_call.arg_count; i++) {
                ast_free(node->as.func_call.args[i]);
            }
            mrt_free(node->as.func_call.args);
            break;

        case AST_IF:
            ast_free(node->as.if_stmt.condition);
            ast_free(node->as.if_stmt.then_branch);
            ast_free(node->as.if_stmt.else_branch);
            break;

        case AST_WHILE:
            ast_free(node->as.while_stmt.condition);
            ast_free(node->as.while_stmt.body);
            break;

        case AST_RETURN:
            ast_free(node->as.return_stmt.value);
            break;

        case AST_SAY:
            ast_free(node->as.say_stmt.value);
            break;

        case AST_BREAK:
        case AST_CONTINUE:
            break;

        case AST_EXPR_STMT:
            ast_free(node->as.expr_stmt.expression);
            break;
    }

    mrt_free(node);
}

/* Helper to convert operator token to readable string */
static const char *op_to_str(TokenType op) {
    switch (op) {
        case TOKEN_PLUS:          return "+";
        case TOKEN_MINUS:         return "-";
        case TOKEN_STAR:          return "*";
        case TOKEN_SLASH:         return "/";
        case TOKEN_PERCENT:       return "%";
        case TOKEN_EQUAL_EQUAL:   return "==";
        case TOKEN_BANG_EQUAL:    return "!=";
        case TOKEN_LESS:          return "<";
        case TOKEN_LESS_EQUAL:    return "<=";
        case TOKEN_GREATER:       return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_AND:           return "and";
        case TOKEN_OR:            return "or";
        case TOKEN_NOT:           return "not";
        default:                  return "?";
    }
}

static void print_node_recursive(const ASTNode *node, const char *prefix, bool is_last);

static void print_children(ASTNode **children, size_t count, const char *prefix) {
    for (size_t i = 0; i < count; i++) {
        char next_prefix[512];
        snprintf(next_prefix, sizeof(next_prefix), "%s", prefix);
        print_node_recursive(children[i], next_prefix, i == count - 1);
    }
}

static void print_node_recursive(const ASTNode *node, const char *prefix, bool is_last) {
    if (!node) return;

    printf("%s%s", prefix, is_last ? "└── " : "├── ");

    char child_prefix[512];
    snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, is_last ? "    " : "│   ");

    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            print_children(node->as.block.statements, node->as.block.count, child_prefix);
            break;

        case AST_BLOCK:
            printf("Block\n");
            print_children(node->as.block.statements, node->as.block.count, child_prefix);
            break;

        case AST_VAR_DECL:
            printf("VarDecl(%s)\n", node->as.var_decl.name);
            if (node->as.var_decl.init) {
                print_node_recursive(node->as.var_decl.init, child_prefix, true);
            }
            break;

        case AST_ASSIGN:
            printf("Assign(%s)\n", node->as.assign.name);
            print_node_recursive(node->as.assign.value, child_prefix, true);
            break;

        case AST_LITERAL:
            switch (node->as.literal.lit_type) {
                case LITERAL_INT:
                    printf("Integer(%ld)\n", (long)node->as.literal.as.int_val);
                    break;
                case LITERAL_FLOAT:
                    printf("Float(%g)\n", node->as.literal.as.float_val);
                    break;
                case LITERAL_STRING:
                    printf("String(\"%s\")\n", node->as.literal.as.string_val);
                    break;
                case LITERAL_BOOL:
                    printf("Boolean(%s)\n", node->as.literal.as.bool_val ? "yes" : "no");
                    break;
                case LITERAL_NONE:
                    printf("None\n");
                    break;
            }
            break;

        case AST_ARRAY_LITERAL:
            printf("ArrayLiteral\n");
            print_children(node->as.array_literal.elements, node->as.array_literal.count, child_prefix);
            break;

        case AST_MAP_LITERAL:
            printf("MapLiteral\n");
            print_children(node->as.map_literal.values, node->as.map_literal.count, child_prefix);
            break;

        case AST_INDEX_GET: {
            printf("IndexGet\n");
            ASTNode *children[2] = { node->as.index_get.target, node->as.index_get.index };
            print_children(children, 2, child_prefix);
            break;
        }

        case AST_INDEX_SET: {
            printf("IndexSet\n");
            ASTNode *children[3] = { node->as.index_set.target, node->as.index_set.index, node->as.index_set.value };
            print_children(children, 3, child_prefix);
            break;
        }

        case AST_IDENTIFIER:
            printf("Identifier(%s)\n", node->as.identifier.name);
            break;

        case AST_BINARY: {
            printf("BinaryExpr(%s)\n", op_to_str(node->as.binary.op));
            ASTNode *children[2] = { node->as.binary.left, node->as.binary.right };
            print_children(children, 2, child_prefix);
            break;
        }

        case AST_UNARY:
            printf("UnaryExpr(%s)\n", op_to_str(node->as.unary.op));
            print_node_recursive(node->as.unary.operand, child_prefix, true);
            break;

        case AST_FUNCTION_DECL: {
            printf("TaskDecl(%s", node->as.func_decl.name);
            if (node->as.func_decl.param_count > 0) {
                printf("(");
                for (size_t i = 0; i < node->as.func_decl.param_count; i++) {
                    printf("%s%s", node->as.func_decl.params[i],
                           i + 1 < node->as.func_decl.param_count ? ", " : "");
                }
                printf(")");
            } else {
                printf("()");
            }
            printf(")\n");
            print_node_recursive(node->as.func_decl.body, child_prefix, true);
            break;
        }

        case AST_FUNCTION_CALL: {
            printf("CallExpr\n");
            size_t total = 1 + node->as.func_call.arg_count;
            ASTNode **children = (ASTNode **)mrt_malloc(total * sizeof(ASTNode *));
            children[0] = node->as.func_call.callee;
            for (size_t i = 0; i < node->as.func_call.arg_count; i++) {
                children[1 + i] = node->as.func_call.args[i];
            }
            print_children(children, total, child_prefix);
            mrt_free(children);
            break;
        }

        case AST_IF: {
            printf("WhenStmt\n");
            if (node->as.if_stmt.else_branch) {
                ASTNode *children[3] = {
                    node->as.if_stmt.condition,
                    node->as.if_stmt.then_branch,
                    node->as.if_stmt.else_branch
                };
                print_children(children, 3, child_prefix);
            } else {
                ASTNode *children[2] = {
                    node->as.if_stmt.condition,
                    node->as.if_stmt.then_branch
                };
                print_children(children, 2, child_prefix);
            }
            break;
        }

        case AST_WHILE: {
            printf("RepeatStmt\n");
            ASTNode *children[2] = {
                node->as.while_stmt.condition,
                node->as.while_stmt.body
            };
            print_children(children, 2, child_prefix);
            break;
        }

        case AST_EACH: {
            printf("EachStmt(%s)\n", node->as.each_stmt.var_name);
            ASTNode *children[2] = {
                node->as.each_stmt.collection,
                node->as.each_stmt.body
            };
            print_children(children, 2, child_prefix);
            break;
        }

        case AST_USE:
            printf("UseStmt(\"%s\")\n", node->as.use_stmt.path);
            break;

        case AST_RETURN:
            printf("GiveStmt\n");
            if (node->as.return_stmt.value) {
                print_node_recursive(node->as.return_stmt.value, child_prefix, true);
            }
            break;

        case AST_SAY:
            printf("SayStmt\n");
            if (node->as.say_stmt.value) {
                print_node_recursive(node->as.say_stmt.value, child_prefix, true);
            }
            break;

        case AST_BREAK:
            printf("BreakStmt\n");
            break;

        case AST_CONTINUE:
            printf("ContinueStmt\n");
            break;

        case AST_EXPR_STMT:
            printf("ExprStmt\n");
            print_node_recursive(node->as.expr_stmt.expression, child_prefix, true);
            break;
    }
}

void ast_print(const ASTNode *node) {
    if (!node) {
        printf("(null)\n");
        return;
    }

    if (node->type == AST_PROGRAM) {
        printf("Program\n");
        for (size_t i = 0; i < node->as.block.count; i++) {
            print_node_recursive(node->as.block.statements[i], "", i == node->as.block.count - 1);
        }
    } else {
        print_node_recursive(node, "", true);
    }
}

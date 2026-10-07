#include "formatter.h"
#include "lexer.h"
#include "parser.h"

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} StrBuf;

static void sb_init(StrBuf *sb) {
    sb->capacity = 128;
    sb->length = 0;
    sb->data = (char *)mrt_malloc(sb->capacity);
    sb->data[0] = '\0';
}

static void sb_append(StrBuf *sb, const char *str) {
    if (!str) return;
    size_t slen = strlen(str);
    if (sb->length + slen + 1 >= sb->capacity) {
        sb->capacity = (sb->capacity + slen + 1) * 2;
        sb->data = (char *)mrt_realloc(sb->data, sb->capacity);
    }
    memcpy(sb->data + sb->length, str, slen);
    sb->length += slen;
    sb->data[sb->length] = '\0';
}

static void sb_indent(StrBuf *sb, int indent_level) {
    for (int i = 0; i < indent_level * 4; i++) {
        sb_append(sb, " ");
    }
}

static const char *format_op_str(TokenType op) {
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
        default:                  return "";
    }
}

static void format_node_recursive(const ASTNode *node, StrBuf *sb, int indent);

static void format_expression(const ASTNode *node, StrBuf *sb) {
    if (!node) return;
    switch (node->type) {
        case AST_LITERAL: {
            char buf[128];
            switch (node->as.literal.lit_type) {
                case LITERAL_INT:
                    snprintf(buf, sizeof(buf), "%ld", (long)node->as.literal.as.int_val);
                    sb_append(sb, buf);
                    break;
                case LITERAL_FLOAT:
                    snprintf(buf, sizeof(buf), "%g", node->as.literal.as.float_val);
                    sb_append(sb, buf);
                    break;
                case LITERAL_STRING:
                    sb_append(sb, "\"");
                    sb_append(sb, node->as.literal.as.string_val);
                    sb_append(sb, "\"");
                    break;
                case LITERAL_BOOL:
                    sb_append(sb, node->as.literal.as.bool_val ? "yes" : "no");
                    break;
                case LITERAL_NONE:
                    sb_append(sb, "none");
                    break;
            }
            break;
        }

        case AST_IDENTIFIER:
            sb_append(sb, node->as.identifier.name);
            break;

        case AST_ARRAY_LITERAL: {
            sb_append(sb, "[");
            for (size_t i = 0; i < node->as.array_literal.count; i++) {
                if (i > 0) sb_append(sb, ", ");
                format_expression(node->as.array_literal.elements[i], sb);
            }
            sb_append(sb, "]");
            break;
        }

        case AST_MAP_LITERAL: {
            sb_append(sb, "{");
            for (size_t i = 0; i < node->as.map_literal.count; i++) {
                if (i > 0) sb_append(sb, ", ");
                sb_append(sb, "\"");
                sb_append(sb, node->as.map_literal.keys[i]);
                sb_append(sb, "\": ");
                format_expression(node->as.map_literal.values[i], sb);
            }
            sb_append(sb, "}");
            break;
        }

        case AST_INDEX_GET:
            format_expression(node->as.index_get.target, sb);
            sb_append(sb, "[");
            format_expression(node->as.index_get.index, sb);
            sb_append(sb, "]");
            break;

        case AST_BINARY: {
            bool need_parens_l = (node->as.binary.left->type == AST_BINARY);
            bool need_parens_r = (node->as.binary.right->type == AST_BINARY);

            if (need_parens_l) sb_append(sb, "(");
            format_expression(node->as.binary.left, sb);
            if (need_parens_l) sb_append(sb, ")");

            sb_append(sb, " ");
            sb_append(sb, format_op_str(node->as.binary.op));
            sb_append(sb, " ");

            if (need_parens_r) sb_append(sb, "(");
            format_expression(node->as.binary.right, sb);
            if (need_parens_r) sb_append(sb, ")");
            break;
        }

        case AST_UNARY:
            sb_append(sb, format_op_str(node->as.unary.op));
            if (node->as.unary.op == TOKEN_NOT) sb_append(sb, " ");
            format_expression(node->as.unary.operand, sb);
            break;

        case AST_FUNCTION_CALL:
            format_expression(node->as.func_call.callee, sb);
            sb_append(sb, "(");
            for (size_t i = 0; i < node->as.func_call.arg_count; i++) {
                if (i > 0) sb_append(sb, ", ");
                format_expression(node->as.func_call.args[i], sb);
            }
            sb_append(sb, ")");
            break;

        case AST_ASSIGN:
            sb_append(sb, node->as.assign.name);
            sb_append(sb, " = ");
            format_expression(node->as.assign.value, sb);
            break;

        case AST_INDEX_SET:
            format_expression(node->as.index_set.target, sb);
            sb_append(sb, "[");
            format_expression(node->as.index_set.index, sb);
            sb_append(sb, "] = ");
            format_expression(node->as.index_set.value, sb);
            break;

        default:
            break;
    }
}

static void format_block(const ASTNode *block, StrBuf *sb, int indent) {
    if (!block || block->type != AST_BLOCK) return;
    for (size_t i = 0; i < block->as.block.count; i++) {
        format_node_recursive(block->as.block.statements[i], sb, indent);
    }
}

static void format_node_recursive(const ASTNode *node, StrBuf *sb, int indent) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < node->as.block.count; i++) {
                format_node_recursive(node->as.block.statements[i], sb, 0);
            }
            break;

        case AST_VAR_DECL:
            sb_indent(sb, indent);
            sb_append(sb, "var ");
            sb_append(sb, node->as.var_decl.name);
            if (node->as.var_decl.init && node->as.var_decl.init->type != AST_LITERAL) {
                sb_append(sb, " = ");
                format_expression(node->as.var_decl.init, sb);
            } else if (node->as.var_decl.init && node->as.var_decl.init->as.literal.lit_type != LITERAL_NONE) {
                sb_append(sb, " = ");
                format_expression(node->as.var_decl.init, sb);
            }
            sb_append(sb, "\n");
            break;

        case AST_ASSIGN:
            sb_indent(sb, indent);
            sb_append(sb, node->as.assign.name);
            sb_append(sb, " = ");
            format_expression(node->as.assign.value, sb);
            sb_append(sb, "\n");
            break;

        case AST_FUNCTION_DECL:
            sb_indent(sb, indent);
            sb_append(sb, "task ");
            sb_append(sb, node->as.func_decl.name);
            sb_append(sb, "(");
            for (size_t i = 0; i < node->as.func_decl.param_count; i++) {
                if (i > 0) sb_append(sb, ", ");
                sb_append(sb, node->as.func_decl.params[i]);
            }
            sb_append(sb, ") {\n");
            format_block(node->as.func_decl.body, sb, indent + 1);
            sb_indent(sb, indent);
            sb_append(sb, "}\n\n");
            break;

        case AST_IF: {
            sb_indent(sb, indent);
            sb_append(sb, "when ");
            format_expression(node->as.if_stmt.condition, sb);
            sb_append(sb, " {\n");
            format_block(node->as.if_stmt.then_branch, sb, indent + 1);
            sb_indent(sb, indent);
            sb_append(sb, "}");

            const ASTNode *cur_else = node->as.if_stmt.else_branch;
            while (cur_else && cur_else->type == AST_IF) {
                sb_append(sb, " otherwise when ");
                format_expression(cur_else->as.if_stmt.condition, sb);
                sb_append(sb, " {\n");
                format_block(cur_else->as.if_stmt.then_branch, sb, indent + 1);
                sb_indent(sb, indent);
                sb_append(sb, "}");
                cur_else = cur_else->as.if_stmt.else_branch;
            }

            if (cur_else) {
                sb_append(sb, " otherwise {\n");
                format_block(cur_else, sb, indent + 1);
                sb_indent(sb, indent);
                sb_append(sb, "}\n");
            } else {
                sb_append(sb, "\n");
            }
            break;
        }

        case AST_WHILE:
            sb_indent(sb, indent);
            sb_append(sb, "repeat ");
            format_expression(node->as.while_stmt.condition, sb);
            sb_append(sb, " {\n");
            format_block(node->as.while_stmt.body, sb, indent + 1);
            sb_indent(sb, indent);
            sb_append(sb, "}\n");
            break;

        case AST_EACH:
            sb_indent(sb, indent);
            sb_append(sb, "each ");
            sb_append(sb, node->as.each_stmt.var_name);
            sb_append(sb, " in ");
            format_expression(node->as.each_stmt.collection, sb);
            sb_append(sb, " {\n");
            format_block(node->as.each_stmt.body, sb, indent + 1);
            sb_indent(sb, indent);
            sb_append(sb, "}\n");
            break;

        case AST_RETURN:
            sb_indent(sb, indent);
            sb_append(sb, "give");
            if (node->as.return_stmt.value) {
                sb_append(sb, " ");
                format_expression(node->as.return_stmt.value, sb);
            }
            sb_append(sb, "\n");
            break;

        case AST_SAY:
            sb_indent(sb, indent);
            sb_append(sb, "say ");
            format_expression(node->as.say_stmt.value, sb);
            sb_append(sb, "\n");
            break;

        case AST_BREAK:
            sb_indent(sb, indent);
            sb_append(sb, "break\n");
            break;

        case AST_CONTINUE:
            sb_indent(sb, indent);
            sb_append(sb, "continue\n");
            break;

        case AST_USE:
            sb_indent(sb, indent);
            sb_append(sb, "use \"");
            sb_append(sb, node->as.use_stmt.path);
            sb_append(sb, "\"\n");
            break;

        case AST_EXPR_STMT:
            sb_indent(sb, indent);
            format_expression(node->as.expr_stmt.expression, sb);
            sb_append(sb, "\n");
            break;

        default:
            break;
    }
}

char *mrt_format_ast(const ASTNode *node) {
    if (!node) return NULL;
    StrBuf sb;
    sb_init(&sb);
    format_node_recursive(node, &sb, 0);
    return sb.data;
}

bool mrt_format_file(const char *filename, bool check_only, bool *out_differs) {
    if (out_differs) *out_differs = false;

    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    if (sz < 0) {
        fclose(f);
        return false;
    }

    char *source = (char *)mrt_malloc((size_t)sz + 1);
    size_t rd = fread(source, 1, (size_t)sz, f);
    source[rd] = '\0';
    fclose(f);

    Lexer lexer;
    lexer_init(&lexer, source, filename);
    TokenArray tokens = lexer_tokenize_all(&lexer);
    if (lexer.had_error) {
        token_array_free(&tokens);
        mrt_free(source);
        return false;
    }

    Parser parser;
    parser_init(&parser, tokens, source, filename);
    ASTNode *ast = parser_parse(&parser);
    if (!ast || parser.had_error) {
        parser_free(&parser);
        mrt_free(source);
        return false;
    }

    char *formatted = mrt_format_ast(ast);
    ast_free(ast);
    parser_free(&parser);

    bool differs = (strcmp(source, formatted) != 0);
    if (out_differs) *out_differs = differs;

    if (!check_only && differs) {
        FILE *out = fopen(filename, "wb");
        if (out) {
            fputs(formatted, out);
            fclose(out);
        }
    }

    mrt_free(source);
    mrt_free(formatted);
    return true;
}

#include "error.h"

const char *error_type_name(ErrorType type) {
    switch (type) {
        case ERR_SYNTAX:  return "SyntaxError";
        case ERR_NAME:    return "NameError";
        case ERR_TYPE:    return "TypeError";
        case ERR_RUNTIME: return "RuntimeError";
        default:          return "Error";
    }
}

static bool s_json_mode = false;
static MrtDiagnostic *s_diagnostics = NULL;
static size_t s_diag_count = 0;
static size_t s_diag_capacity = 0;

void mrt_diagnostics_init(void) {
    mrt_diagnostics_free();
    s_json_mode = false;
}

void mrt_diagnostics_set_json_mode(bool enable) {
    s_json_mode = enable;
}

bool mrt_diagnostics_is_json_mode(void) {
    return s_json_mode;
}

void mrt_diagnostics_add(const char *file, int line, int column,
                         const char *severity, const char *type,
                         const char *message) {
    if (s_diag_count >= s_diag_capacity) {
        size_t new_cap = s_diag_capacity == 0 ? 8 : s_diag_capacity * 2;
        MrtDiagnostic *new_items = (MrtDiagnostic *)mrt_realloc(s_diagnostics, new_cap * sizeof(MrtDiagnostic));
        s_diagnostics = new_items;
        s_diag_capacity = new_cap;
    }

    MrtDiagnostic *d = &s_diagnostics[s_diag_count++];
    d->file = file ? mrt_strdup(file) : mrt_strdup("<unknown>");
    d->message = message ? mrt_strdup(message) : mrt_strdup("");
    d->type = type ? mrt_strdup(type) : mrt_strdup("Error");
    d->severity = severity ? mrt_strdup(severity) : mrt_strdup("error");
    d->line = line;
    d->column = column;
}

int mrt_diagnostics_count(void) {
    return (int)s_diag_count;
}

static void print_json_escaped(FILE *out, const char *str) {
    if (!str) return;
    for (const char *p = str; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\b': fputs("\\b", out); break;
            case '\f': fputs("\\f", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if ((unsigned char)*p < 0x20) {
                    fprintf(out, "\\u%04x", (unsigned char)*p);
                } else {
                    fputc(*p, out);
                }
                break;
        }
    }
}

void mrt_diagnostics_print_json(FILE *out) {
    if (!out) out = stdout;
    fprintf(out, "{\n  \"diagnostics\": [");
    for (size_t i = 0; i < s_diag_count; i++) {
        MrtDiagnostic *d = &s_diagnostics[i];
        fprintf(out, "%s\n    {\n", i > 0 ? "," : "");
        fprintf(out, "      \"severity\": \"");
        print_json_escaped(out, d->severity);
        fprintf(out, "\",\n      \"type\": \"");
        print_json_escaped(out, d->type);
        fprintf(out, "\",\n      \"file\": \"");
        print_json_escaped(out, d->file);
        fprintf(out, "\",\n      \"line\": %d,\n", d->line);
        fprintf(out, "      \"column\": %d,\n", d->column);
        fprintf(out, "      \"message\": \"");
        print_json_escaped(out, d->message);
        fprintf(out, "\"\n    }");
    }
    if (s_diag_count > 0) {
        fprintf(out, "\n  ");
    }
    fprintf(out, "]\n}\n");
}

void mrt_diagnostics_free(void) {
    if (s_diagnostics) {
        for (size_t i = 0; i < s_diag_count; i++) {
            mrt_free(s_diagnostics[i].file);
            mrt_free(s_diagnostics[i].message);
            mrt_free(s_diagnostics[i].type);
            mrt_free(s_diagnostics[i].severity);
        }
        mrt_free(s_diagnostics);
        s_diagnostics = NULL;
    }
    s_diag_count = 0;
    s_diag_capacity = 0;
}

void mrt_report_error(const char *filename, const char *source,
                      int line, int column, ErrorType type,
                      const char *fmt, ...) {
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    const char *fn = filename ? filename : "<source>";

    if (s_json_mode) {
        mrt_diagnostics_add(fn, line, column, "error", error_type_name(type), message);
        return;
    }

    if (source && line > 0) {
        /* Find the start and end of the requested line */
        const char *cur = source;
        int cur_line = 1;
        while (*cur && cur_line < line) {
            if (*cur == '\n') {
                cur_line++;
            }
            cur++;
        }

        const char *line_start = cur;
        while (*cur && *cur != '\n' && *cur != '\r') {
            cur++;
        }
        size_t line_len = (size_t)(cur - line_start);

        fprintf(stderr, "\n%s:%d:%d\n\n", fn, line, column);

        /* Print the source line */
        fprintf(stderr, "%.*s\n", (int)line_len, line_start);

        /* Print the caret indicator */
        int col = column > 0 ? column : 1;
        for (int i = 1; i < col; i++) {
            /* If original character was tab, match tab or spaces */
            if (i - 1 < (int)line_len && line_start[i - 1] == '\t') {
                fputc('\t', stderr);
            } else {
                fputc(' ', stderr);
            }
        }
        fprintf(stderr, "^\n\n");
        fprintf(stderr, "%s: %s\n\n", error_type_name(type), message);
    } else if (line > 0 && column > 0) {
        fprintf(stderr, "%s:%d:%d: %s: %s\n", fn, line, column, error_type_name(type), message);
    } else {
        fprintf(stderr, "%s: %s\n", error_type_name(type), message);
    }
}

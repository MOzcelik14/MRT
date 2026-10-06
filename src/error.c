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

void mrt_report_error(const char *filename, const char *source,
                      int line, int column, ErrorType type,
                      const char *fmt, ...) {
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    const char *fn = filename ? filename : "<source>";

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

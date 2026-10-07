#ifndef MRT_ERROR_H
#define MRT_ERROR_H

#include "common.h"

typedef enum {
    ERR_SYNTAX,
    ERR_NAME,
    ERR_TYPE,
    ERR_RUNTIME
} ErrorType;

typedef struct {
    char *file;
    char *message;
    char *type;
    char *severity;
    int line;
    int column;
} MrtDiagnostic;

const char *error_type_name(ErrorType type);

void mrt_diagnostics_init(void);
void mrt_diagnostics_set_json_mode(bool enable);
bool mrt_diagnostics_is_json_mode(void);
void mrt_diagnostics_add(const char *file, int line, int column,
                         const char *severity, const char *type,
                         const char *message);
int mrt_diagnostics_count(void);
void mrt_diagnostics_print_json(FILE *out);
void mrt_diagnostics_free(void);

void mrt_report_error(const char *filename, const char *source,
                      int line, int column, ErrorType type,
                      const char *fmt, ...);

#endif /* MRT_ERROR_H */

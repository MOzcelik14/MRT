#ifndef MRT_ERROR_H
#define MRT_ERROR_H

#include "common.h"

typedef enum {
    ERR_SYNTAX,
    ERR_NAME,
    ERR_TYPE,
    ERR_RUNTIME
} ErrorType;

const char *error_type_name(ErrorType type);

void mrt_report_error(const char *filename, const char *source,
                      int line, int column, ErrorType type,
                      const char *fmt, ...);

#endif /* MRT_ERROR_H */

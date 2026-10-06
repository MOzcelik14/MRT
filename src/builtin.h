#ifndef MRT_BUILTIN_H
#define MRT_BUILTIN_H

#include "common.h"
#include "value.h"
#include "environment.h"

void builtin_register_all(Environment *env);

#endif /* MRT_BUILTIN_H */

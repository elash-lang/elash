#pragma once

#include <elash/defs/int-types.h>

typedef struct ElSourceLocInfo {
    uint line;
    const char* file;
    const char* func;
} ElSourceLocInfo;

#define EL_SRCLOC_INFO    \
    ((ElSourceLocInfo) {  \
        .line = __LINE__, \
        .file = __FILE__, \
        .func = __func__, \
    })

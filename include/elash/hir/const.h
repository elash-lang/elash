#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct ElHirConstant {
    // tagged by the type field of Expr
    union {
        int64_t int_; // TODO: use something like bigint here
        char    char_;
        bool    bool_;
    } as;
} ElHirConstant;

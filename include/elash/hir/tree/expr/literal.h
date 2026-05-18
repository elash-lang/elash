#pragma once

#include <stdint.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

typedef struct ElHirLiteral {
    // tagged by the type field of Expr
    union {
        int64_t int_;
        uint64_t uint_;
        char char_;
        bool bool_;
    } as;
} ElHirLiteral;

ElHirExpr* el_hir_new_int_literal(ElDynArena* arena, ElType* type, int64_t value);
ElHirExpr* el_hir_new_uint_literal(ElDynArena* arena, ElType* type, uint64_t value);
ElHirExpr* el_hir_new_char_literal(ElDynArena* arena, ElType* type, char value);
ElHirExpr* el_hir_new_bool_literal(ElDynArena* arena, ElType* type, bool value);

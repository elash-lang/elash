#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

typedef struct ElHirArrayLit {
    ElHirExpr** values;
    usize count;
} ElHirArrayLit;

ElHirExpr* el_hir_new_array_lit(ElDynArena* arena, ElType* type, ElHirExpr** values, usize count);

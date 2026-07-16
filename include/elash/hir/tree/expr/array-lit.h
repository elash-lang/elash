#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef struct ElHirArrayLit {
    ElHirExpr** values;
    usize count;
} ElHirArrayLit;

ElHirExpr* el_hir_new_array_lit(ElDynArena* arena, ElHirType* type, ElHirExpr** values, usize count);

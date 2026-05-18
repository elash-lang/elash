#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

typedef struct ElHirCallExpr {
    ElHirExpr* callee;
    ElHirExpr** args;
    usize arg_count;
} ElHirCallExpr;

ElHirExpr* el_hir_new_call_expr(
    ElDynArena* arena, ElType* type, ElHirExpr* callee, ElHirExpr** args, usize arg_count
);

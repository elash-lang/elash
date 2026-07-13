#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

typedef struct ElHirCastExpr {
    ElHirExpr* expr;
    // NOTE: type is stored in the ElHirExpr struct
    //ElType* type;
} ElHirCastExpr;

ElHirExpr* el_hir_new_cast_expr(ElDynArena* arena, ElType* type, ElHirExpr* expr);

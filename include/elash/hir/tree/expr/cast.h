#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef struct ElHirCastExpr {
    ElHirExpr* expr;
    // NOTE: type is stored in the ElHirExpr struct
    //ElHirType* type;
} ElHirCastExpr;

ElHirExpr* el_hir_new_cast_expr(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElHirExpr* expr);

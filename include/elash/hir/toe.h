#pragma once

#include <elash/hir/tree/expr.h>
#include <elash/hir/type.h>

typedef struct ElHirToE {
    bool is_type; // NOTE: maybe an enum would be better. but who cares.
    union {
        ElHirType* type;
        ElHirExpr* expr;
    } as;
} ElHirToE;

ElHirToE* el_hir_new_toe_type(ElDynArena* arena, ElHirType* type);
ElHirToE* el_hir_new_toe_expr(ElDynArena* arena, ElHirExpr* expr);


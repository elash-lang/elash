#include <elash/hir/toe.h>

ElHirToE* el_hir_new_toe_type(ElDynArena* arena, ElHirType* type) {
    ElHirToE* node = EL_DYNARENA_NEW_STRUCT(arena, ElHirToE, {
        .is_type = true,
        .as.type = type,
    });
    return node;
}

ElHirToE* el_hir_new_toe_expr(ElDynArena* arena, ElHirExpr* expr) {
    ElHirToE* node = EL_DYNARENA_NEW_STRUCT(arena, ElHirToE, {
        .is_type = false,
        .as.expr = expr,
    });
    return node;
}


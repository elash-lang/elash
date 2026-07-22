#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_member_expr(ElDynArena* arena, ElHirType* type, ElHirExpr* expr, ElStringView name, usize index) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_MEMBER,
        .type = type,
        .as.member = {
            .expr = expr,
            .name = name,
            .index = index,
        },
    });
}

ElHirExpr* el_hir_new_tmember_expr(ElDynArena* arena, ElHirType* type, ElHirExpr* expr, usize index) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_TMEMBER,
        .type = type,
        .as.tmember = {
            .expr = expr,
            .index = index,
        },
    });
}

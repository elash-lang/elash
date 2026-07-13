#include <elash/hir/tree/expr/cast.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_cast_expr(ElDynArena* arena, ElType* type, ElHirExpr* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_CAST,
        .type = type,
        .as.cast.expr = expr,
    });
}


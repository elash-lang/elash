#include <elash/hir/tree/expr/array-lit.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_array_lit(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElHirExpr** values, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_ARRAYLIT,
        .type = type,
        .span = span,
        .as.array_lit = {
            .values = values,
            .count  = count,
        },
    });
}

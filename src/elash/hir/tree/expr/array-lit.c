#include <elash/hir/tree/expr/array-lit.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_array_lit(ElDynArena* arena, ElType* type, ElHirExpr** values, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_ARRAY_LITERAL,
        .type = type,
        .as.array_lit = {
            .values = values,
            .count  = count,
        },
    });
}

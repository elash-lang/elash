#include <elash/hir/tree/expr/array-lit.h>
#include <elash/hir/tree/expr.h>

ElHirExprNode* el_hir_new_array_lit(ElDynArena* arena, ElType* type, ElHirExprNode** values, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExprNode, {
        .kind = EL_HIR_EXPR_ARRAY_LITERAL,
        .type = type,
        .as.array_lit = {
            .values = values,
            .count  = count,
        },
    });
}

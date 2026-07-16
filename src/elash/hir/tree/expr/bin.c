#include <elash/hir/tree/expr/bin.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_bin_expr(ElDynArena* arena, ElHirType* type, ElSemaBinOp op, ElHirExpr* left, ElHirExpr* right) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_BINARY,
        .type = type,
        .as.binary = {
            .left = left,
            .op = op,
            .right = right,
        }
    });
}

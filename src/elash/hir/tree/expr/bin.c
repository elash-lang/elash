#include <elash/hir/tree/expr/bin.h>
#include <elash/hir/tree/expr.h>

ElHirExprNode* el_hir_new_bin_expr(ElDynArena* arena, ElType* type, ElSemaBinOp op, ElHirExprNode* left, ElHirExprNode* right) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExprNode, {
        .kind = EL_HIR_EXPR_BINARY,
        .type = type,
        .as.binary = {
            .left = left,
            .op = op,
            .right = right,
        }
    });
}

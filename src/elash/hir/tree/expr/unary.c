#include <elash/hir/tree/expr/unary.h>
#include <elash/hir/tree/expr.h>

ElHirExprNode* el_hir_new_unary_expr(ElDynArena* arena, ElType* type, ElSemaUnaryOp op, ElHirExprNode* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExprNode, {
        .kind = EL_HIR_EXPR_UNARY,
        .type = type,
        .as.unary = {
            .op = op,
            .operand = operand,
        },
    });
}

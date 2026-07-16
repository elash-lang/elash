#include <elash/hir/tree/expr/unary.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_unary_expr(ElDynArena* arena, ElHirType* type, ElSemaUnaryOp op, ElHirExpr* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_UNARY,
        .type = type,
        .as.unary = {
            .op = op,
            .operand = operand,
        },
    });
}

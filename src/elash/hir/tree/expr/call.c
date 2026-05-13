#include <elash/hir/tree/expr/call.h>
#include <elash/hir/tree/expr.h>

ElHirExprNode* el_hir_new_call_expr(
    ElDynArena* arena, ElType* type, ElHirExprNode* callee, ElHirExprNode** args, usize arg_count
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExprNode, {
        .kind = EL_HIR_EXPR_CALL,
        .type = type,
        .as.call = (ElHirCallExprNode) {
            .callee = callee,
            .args = args,
            .arg_count = arg_count,
        }
    });
}

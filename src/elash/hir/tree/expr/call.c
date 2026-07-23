#include <elash/hir/tree/expr/call.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_call_expr(
    ElDynArena* arena, ElSourceSpan span, ElHirType* type,
    ElHirExpr* callee, ElHirExpr** args, usize arg_count
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_CALL,
        .type = type,
        .span = span,
        .as.call = (ElHirCallExpr) {
            .callee = callee,
            .args = args,
            .arg_count = arg_count,
        }
    });
}

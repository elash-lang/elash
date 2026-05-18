#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_compound_assign_stmt(
    ElDynArena* arena, ElSemaBinOp op, ElHirExpr* target, ElHirExpr* value
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_COMPOUND_ASSIGN,
        .next = NULL,
        .as.cassign = {
            .op = op,
            .target = target,
            .value = value,
        },
    });
}

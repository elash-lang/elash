#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_assign_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* target, ElHirExpr* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_ASSIGN,
        .span = span,
        .next = NULL,
        .as.assign = {
            .target = target,
            .value = value,
        },
    });
}

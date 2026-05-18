#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_assign_stmt(ElDynArena* arena, ElHirExpr* target, ElHirExpr* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_ASSIGN,
        .next = NULL,
        .as.assign = {
            .target = target,
            .value = value,
        },
    });
}

#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_while_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* cond, ElHirStmt* body) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_WHILE,
        .span = span,
        .next = NULL,
        .as.while_ = {
            .cond = cond,
            .body = body,
        },
    });
}

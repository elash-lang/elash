#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_if_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* cond, ElHirStmt* then, ElHirStmt* else_) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_IF,
        .span = span,
        .next = NULL,
        .as.if_ = {
            .cond = cond,
            .then = then,
            .else_ = else_,
        },
    });
}

#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_break_stmt(ElDynArena* arena, ElSourceSpan span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_BREAK,
        .span = span,
        .next = NULL,
        .as.break_ = {},
    });
}

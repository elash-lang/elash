#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_break_stmt(ElDynArena* arena) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_BREAK,
        .next = NULL,
        .as.break_ = {},
    });
}


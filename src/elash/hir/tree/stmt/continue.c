#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_continue_stmt(ElDynArena* arena) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_CONTINUE,
        .next = NULL,
        .as.continue_ = {},
    });
}


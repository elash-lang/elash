#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_while_stmt(ElDynArena* arena, ElHirExprNode* cond, ElHirStmtNode* body) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_WHILE,
        .next = NULL,
        .as.while_ = {
            .cond = cond,
            .body = body,
        },
    });
}


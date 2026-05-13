#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_if_stmt(ElDynArena* arena, ElHirExprNode* cond, ElHirStmtNode* then, ElHirStmtNode* else_) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_IF,
        .next = NULL,
        .as.if_ = {
            .cond = cond,
            .then = then,
            .else_ = else_,
        },
    });
}

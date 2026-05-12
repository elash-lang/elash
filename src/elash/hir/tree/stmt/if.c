#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_if_stmt(ElDynArena* arena, ElHirExprNode* cond, ElHirStmtNode* then, ElHirStmtNode* else_) {
    ElHirStmtNode* node = EL_DYNARENA_NEW(arena, ElHirStmtNode);
    *node = (ElHirStmtNode) {
        .kind = EL_HIR_STMT_IF,
        .next = NULL,
        .as.if_ = {
            .cond = cond,
            .then = then,
            .else_ = else_,
        },
    };
    return node;
}

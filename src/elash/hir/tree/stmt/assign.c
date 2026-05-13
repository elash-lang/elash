#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_assign_stmt(ElDynArena* arena, ElHirExprNode* target, ElHirExprNode* value) {
    ElHirStmtNode* node = EL_DYNARENA_NEW(arena, ElHirStmtNode);
    *node = (ElHirStmtNode) {
        .kind = EL_HIR_STMT_ASSIGN,
        .next = NULL,
        .as.assign = {
            .target = target,
            .value = value,
        },
    };
    return node;
}

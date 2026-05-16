#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_cassign_stmt(
    ElDynArena* arena, ElSemaBinOp op, ElHirExprNode* target, ElHirExprNode* value
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_COMPOUND_ASSIGN,
        .next = NULL,
        .as.cassign = {
            .op = op,
            .target = target,
            .value = value,
        },
    });
}

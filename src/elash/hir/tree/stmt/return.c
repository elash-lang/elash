#include <elash/hir/tree/stmt/return.h>
#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_return_stmt(ElDynArena* arena, ElHirExprNode* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_RETURN,
        .next = NULL,
        .as.return_ = {
            .value = value,
        },
    });
}

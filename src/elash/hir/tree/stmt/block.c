#include <elash/hir/tree/stmt/block.h>
#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_block_stmt(ElDynArena* arena, ElHirStmtNode* stmts) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_BLOCK,
        .next = NULL,
        .as.block = {
            .stmts = stmts,
        }
    });
}

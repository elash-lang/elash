#include <elash/hir/tree/stmt/block.h>
#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_block_stmt(ElDynArena* arena, ElSourceSpan span, ElHirStmt* stmts) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_BLOCK,
        .span = span,
        .next = NULL,
        .as.block = {
            .stmts = stmts,
        }
    });
}

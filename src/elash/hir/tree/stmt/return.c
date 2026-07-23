#include <elash/hir/tree/stmt/return.h>
#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_return_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_RETURN,
        .span = span,
        .next = NULL,
        .as.return_ = {
            .value = value,
        },
    });
}

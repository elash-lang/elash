#include "elash/srcdoc/span.h"
#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_continue_stmt(ElDynArena* arena, ElSourceSpan span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_CONTINUE,
        .span = span,
        .next = NULL,
        .as.continue_ = {},
    });
}

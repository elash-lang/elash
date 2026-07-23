#include <elash/hir/tree/stmt.h>

#include <stddef.h>

ElHirStmt* el_hir_new_expr_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_EXPR,
        .span = span,
        .next = NULL,
        .as.expr = expr,
    });
}

ElHirStmt* el_hir_new_decl_stmt(ElDynArena* arena, ElSourceSpan span, ElHirDecl* decl) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_DECL,
        .span = span,
        .next = NULL,
        .as.decl = decl,
    });
}

void el_hir_stmt_list_append(ElHirStmt** head, ElHirStmt** tail, ElHirStmt* stmt) {
    if (*head == NULL) {
        *head = stmt;
        *tail = stmt;
    } else {
        (*tail)->next = stmt;
        *tail = stmt;
    }
}

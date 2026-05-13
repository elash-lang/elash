#include <elash/hir/tree/stmt.h>

#include <stddef.h>

ElHirStmtNode* el_hir_new_expr_stmt(ElDynArena* arena, ElHirExprNode* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_EXPR,
        .next = NULL,
        .as.expr = expr,
    });
}

void el_hir_stmt_list_append(ElHirStmtNode** head, ElHirStmtNode** tail, ElHirStmtNode* stmt) {
    if (*head == NULL) {
        *head = stmt;
        *tail = stmt;
    } else {
        (*tail)->next = stmt;
        *tail = stmt;
    }
}

#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_expr_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_EXPR,
        .span = span,
        .as.expr = expr,
        .next = NULL,
    });
}

void el_ast_stmt_list_append(ElAstStmt** head, ElAstStmt** tail, ElAstStmt* stmt) {
    if (*head == NULL) {
        *head = stmt;
        *tail = stmt;
    } else {
        (*tail)->next = stmt;
        *tail = stmt;
    }
}

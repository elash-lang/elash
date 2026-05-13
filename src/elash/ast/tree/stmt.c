#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_expr_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_EXPR,
        .span = span,
        .as.expr = expr,
        .next = NULL,
    });
}

void el_ast_stmt_list_append(ElAstStmtNode** head, ElAstStmtNode** tail, ElAstStmtNode* stmt) {
    if (*head == NULL) {
        *head = stmt;
        *tail = stmt;
    } else {
        (*tail)->next = stmt;
        *tail = stmt;
    }
}

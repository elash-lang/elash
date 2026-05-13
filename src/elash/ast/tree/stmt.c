#include <elash/ast/tree/stmt.h>

ElAstStmtNode el_ast_expr_stmt(ElSourceSpan span, ElAstExprNode* expr) {
    return (ElAstStmtNode) {
        .type = EL_AST_STMT_EXPR,
        .span = span,
        .as.expr = expr,
        .next = NULL,
    };
}

ElAstStmtNode* el_ast_new_expr_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* expr) {
    ElAstStmtNode* node = EL_DYNARENA_NEW(arena, ElAstStmtNode);
    *node = el_ast_expr_stmt(span, expr);
    return node;
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

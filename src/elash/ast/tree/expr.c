#include <elash/ast/tree/expr.h>

#include <inttypes.h>

void el_ast_expr_list_append(ElAstExprNode** head, ElAstExprNode** tail, ElAstExprNode* expr) {
    if (*head == NULL) {
        *head = expr;
        *tail = expr;
    } else {
        (*tail)->next = expr;
        *tail = expr;
    }
}

#include <elash/ast/tree/expr.h>

#include <inttypes.h>

void el_ast_expr_list_append(ElAstExpr** head, ElAstExpr** tail, ElAstExpr* expr) {
    if (*head == NULL) {
        *head = expr;
        *tail = expr;
    } else {
        (*tail)->next = expr;
        *tail = expr;
    }
}

#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_if_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExprNode* cond, ElAstStmtNode* then, ElAstStmtNode* else_
) {
    ElAstStmtNode* node = EL_DYNARENA_NEW(arena, ElAstStmtNode);
    *node = (ElAstStmtNode) {
        .type = EL_AST_STMT_IF,
        .span = span,
        .next = NULL,
        .as.if_ = {
            .cond  = cond,
            .then  = then,
            .else_ = else_,
        },
    };
    return node;
}

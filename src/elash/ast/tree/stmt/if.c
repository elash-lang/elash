#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_if_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExprNode* cond, ElAstStmtNode* then, ElAstStmtNode* else_
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_IF,
        .span = span,
        .next = NULL,
        .as.if_ = {
            .cond  = cond,
            .then  = then,
            .else_ = else_,
        },
    });
}

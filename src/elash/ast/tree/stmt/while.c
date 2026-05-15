#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_while_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExprNode* cond, ElAstStmtNode* body
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_WHILE,
        .span = span,
        .next = NULL,
        .as.while_ = {
            .cond  = cond,
            .body  = body,
        },
    });
}


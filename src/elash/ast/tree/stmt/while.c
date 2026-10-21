#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_while_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstStmt* init, ElAstExpr* cond, ElAstStmt* body, ElAstStmt* post
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_WHILE,
        .span = span,
        .next = NULL,
        .as.while_ = {
            .init = init,
            .cond = cond,
            .body = body,
            .post = post,
        },
    });
}

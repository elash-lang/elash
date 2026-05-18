#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_if_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExpr* cond, ElAstStmt* then, ElAstStmt* else_
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
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

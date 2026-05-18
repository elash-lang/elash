#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_return_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_RETURN,
        .span = span,
        .next = NULL,
        .as.return_ = {
            .value = value,
        },
    });
}

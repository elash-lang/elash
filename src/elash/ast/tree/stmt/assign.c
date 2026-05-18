#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_assign_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* target, ElAstExpr* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_ASSIGN,
        .span = span,
        .next = NULL,
        .as.assign = {
            .target = target,
            .value  = value,
        },
    });
}

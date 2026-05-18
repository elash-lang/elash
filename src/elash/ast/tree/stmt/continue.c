#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_continue_stmt(ElDynArena* arena, ElSourceSpan span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_CONTINUE,
        .span = span,
        .next = NULL,
        .as.continue_ = {},
    });
}

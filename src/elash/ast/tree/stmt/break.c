#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_break_stmt(ElDynArena* arena, ElSourceSpan span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_BREAK,
        .span = span,
        .next = NULL,
        .as.break_ = {},
    });
}

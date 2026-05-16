#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_continue_stmt(ElDynArena* arena, ElSourceSpan span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_CONTINUE,
        .span = span,
        .next = NULL,
        .as.continue_ = {},
    });
}


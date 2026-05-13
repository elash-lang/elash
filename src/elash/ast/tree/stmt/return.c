#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_return_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_RETURN,
        .span = span,
        .next = NULL,
        .as.return_ = {
            .value = value,
        },
    });
}

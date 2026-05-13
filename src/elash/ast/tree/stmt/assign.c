#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_assign_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* target, ElAstExprNode* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_ASSIGN,
        .span = span,
        .next = NULL,
        .as.assign = {
            .target = target,
            .value  = value,
        },
    });
}

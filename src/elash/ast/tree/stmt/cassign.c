#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_compound_assign_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElSemaBinOp op, ElAstExprNode* target, ElAstExprNode* value
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_COMPOUND_ASSIGN,
        .span = span,
        .next = NULL,
        .as.cassign = {
            .op = op,
            .target = target,
            .value  = value,
        },
    });
}


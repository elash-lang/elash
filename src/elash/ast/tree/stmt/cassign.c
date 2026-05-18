#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_compound_assign_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElSemaBinOp op, ElAstExpr* target, ElAstExpr* value
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
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

#include <elash/ast/tree/stmt.h>

ElAstStmtNode* el_ast_new_var_def_stmt(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* type, ElAstIdentNode* name, ElAstExprNode* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_VAR_DEF,
        .span = span,
        .next = NULL,
        .as.var_def = {
            .name = name,
            .type = type,
            .init = init,
        },
    });
}

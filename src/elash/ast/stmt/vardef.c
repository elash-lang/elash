#include <elash/ast/stmt.h>

ElAstStmtNode* el_ast_new_var_def_stmt(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* type, ElAstIdentNode* name, ElAstExprNode* init) {
    ElAstStmtNode* node = EL_DYNARENA_NEW(arena, ElAstStmtNode);
    *node = (ElAstStmtNode) {
        .type = EL_AST_STMT_VAR_DEF,
        .span = span,
        .next = NULL,
        .as.var_def = {
            .name = name,
            .type = type,
            .init = init,
        },
    };
    return node;
}

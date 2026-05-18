#include <elash/ast/tree/stmt.h>

ElAstStmt* el_ast_new_var_def_stmt(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name, ElAstInit* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
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

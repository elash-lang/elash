#include <elash/ast/tree/common/decl.h>

ElAstDecl* el_ast_new_var_def(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name, ElAstInit* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_VAR,
        .span = span,
        .as.var = {
            .name = name,
            .type = type,
            .init = init,
        },
    });
}

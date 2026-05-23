#include <elash/ast/tree/common/decl.h>

ElAstDecl* el_ast_new_var_decl(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_VAR_DECL,
        .span = span,
        .as.var_decl = {
            .type = type,
            .name = name,
        },
    });
}

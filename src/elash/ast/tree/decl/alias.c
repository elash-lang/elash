#include <elash/ast/tree/decl.h>

ElAstDecl* el_ast_new_alias(ElDynArena* arena, ElSourceSpan span, ElStringView name, ElAstToE target) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_ALIAS,
        .span = span,
        .as.alias = {
            .name = name,
            .target = target,
        },
    });
}

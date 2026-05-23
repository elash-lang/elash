#include <elash/ast/tree/toplevel.h>

ElAstTopLevel* el_ast_new_toplevel_decl(ElDynArena* arena, ElAstDecl* decl) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTopLevel, {
        .type = EL_AST_TOPLVL_DECL,
        .span = decl->span,
        .next = NULL,
        .as.decl = decl,
    });
}

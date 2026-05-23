#include <elash/hir/tree/toplevel.h>

ElHirTopLevel* el_hir_new_toplevel_decl(ElDynArena* arena, ElHirDecl* decl) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirTopLevel, {
        .kind = EL_HIR_TOPLVL_DECL,
        .as.decl = decl,
        .next = NULL,
    });
}

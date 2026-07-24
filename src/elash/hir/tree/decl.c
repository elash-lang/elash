#include <elash/hir/tree/decl.h>

ElHirDecl* el_hir_decl_none(ElDynArena* arena, ElSourceSpan span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirDecl, {
        .kind = EL_HIR_DECL_NONE,
        .span = span,
    });
}

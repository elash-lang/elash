#include <elash/hir/tree/decl.h>

ElHirDecl* el_hir_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElHirSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirDecl, {
        .kind = EL_HIR_DECL_FUNC_DECL,
        .span = span,
        .as.func_decl = {
            .symbol = symbol,
        },
    });
}

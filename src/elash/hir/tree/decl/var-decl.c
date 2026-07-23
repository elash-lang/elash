#include <elash/hir/tree/decl.h>

ElHirDecl* el_hir_new_var_decl(ElDynArena* arena, ElSourceSpan span, ElHirSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirDecl, {
        .kind = EL_HIR_DECL_VAR_DECL,
        .span = span,
        .as.var_decl = {
            .var = symbol,
        },
    });
}

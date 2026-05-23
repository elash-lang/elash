#include <elash/hir/tree/decl.h>

ElHirDecl* el_hir_new_func_decl(ElDynArena* arena, ElSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirDecl, {
        .kind = EL_HIR_DECL_FUNC_DECL,
        .as.func_decl = {
            .symbol = symbol,
        },
    });
}

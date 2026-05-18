#include <elash/hir/tree/toplevel.h>

#include <stddef.h>

ElHirTopLevel* el_hir_new_func_decl(ElDynArena* arena, ElSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirTopLevel, {
        .kind = EL_HIR_TOPLVL_FUNC_DECL,
        .next = NULL,
        .as.func_decl.symbol = symbol,
    });
}

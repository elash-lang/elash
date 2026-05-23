#include <elash/hir/tree/decl.h>

ElHirDecl* el_hir_new_func_def(ElDynArena* arena, ElSymbol* symbol, ElHirBlockStmt block) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirDecl, {
        .kind = EL_HIR_DECL_FUNC_DEF,
        .as.func_def = {
            .symbol = symbol,
            .block = block,
        },
    });
}

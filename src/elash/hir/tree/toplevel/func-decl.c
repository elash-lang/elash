#include <elash/hir/tree/toplevel.h>

#include <stddef.h>

ElHirTopLevelNode* el_hir_new_func_decl(ElDynArena* arena, ElSymbol* symbol) {
    ElHirTopLevelNode* node = EL_DYNARENA_NEW(arena, ElHirTopLevelNode);
    *node = (ElHirTopLevelNode) {
        .kind = EL_HIR_TOPLVL_FUNC_DECL,
        .next = NULL,
        .as.func_decl.symbol = symbol,
    };
    return node;
}

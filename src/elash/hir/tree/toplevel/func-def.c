#include <elash/hir/tree/toplevel.h>

#include <stddef.h>

ElHirTopLevelNode* el_hir_new_func_def(ElDynArena* arena, ElSymbol* symbol, ElHirBlockStmtNode block) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirTopLevelNode, {
        .kind = EL_HIR_TOPLVL_FUNC_DEF,
        .next = NULL,
        .as.func_def = {
            .symbol = symbol,
            .block = block,
        },
    });
}

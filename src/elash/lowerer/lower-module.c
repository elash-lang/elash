#include <elash/lowerer/lowerer.h>
#include <elash/mir/module.h>

ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir) {
    lw->current_mod = el_mir_new_module(lw->arena);
    lw->symbol_map = EL_DYNARENA_NEW_ARR_ZEROED(lw->arena, ElMirValue*, hir->sym_count);

    for (ElHirTopLevel* node = hir->head; node != NULL; node = node->next) {
        el_lowerer_lower_toplvl(lw, node);
    }

    return lw->current_mod;
}

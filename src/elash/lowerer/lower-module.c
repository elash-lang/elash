#include "lowerer-internals.h"

#include <elash/mir/module.h>

ElMirModule* el_lower_module(ElLowerer* lw, ElHirModule* hir) {
    lw->current_mod = el_mir_new_module(lw->arena);
    lw->symbol_map = EL_DYNARENA_NEW_ARR_ZEROED(lw->arena, ElMirValue*, hir->sym_count);
    lw->mir_symbol_map = EL_DYNARENA_NEW_ARR_ZEROED(lw->arena, ElMirSymbol*, hir->sym_count);
    lw->next_sym_id = hir->sym_count;

    for (ElHirDecl* decl = hir->head; decl != NULL; decl = decl->next) {
        el_lower_global_decl(lw, decl);
    }

    lw->current_mod->sym_count = lw->next_sym_id;
    return lw->current_mod;
}

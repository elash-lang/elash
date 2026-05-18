#include <elash/hir/tree/module.h>
#include <stddef.h>

ElHirModule* el_hir_new_module(ElDynArena* arena) {
    ElHirModule* mod = EL_DYNARENA_NEW(arena, ElHirModule);
    mod->head = NULL;
    mod->tail = NULL;
    mod->count = 0;
    return mod;
}

void el_hir_module_append(ElHirModule* mod, ElHirTopLevel* node) {
    if (mod->head == NULL) {
        mod->head = node;
        mod->tail = node;
    } else {
        mod->tail->next = node;
        mod->tail = node;
    }
    mod->count++;
}

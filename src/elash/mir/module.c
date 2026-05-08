#include <elash/mir/module.h>
#include <elash/util/dynarena.h>

#include <stddef.h>

ElMirModule* el_mir_new_module(ElDynArena* arena) {
    ElMirModule* mod = EL_DYNARENA_NEW(arena, ElMirModule);
    mod->first_func = NULL;
    mod->last_func = NULL;
    mod->func_count = 0;

    return mod;
}

void el_mir_module_add_func(ElMirModule* mod, ElMirFunc* func) {
    if (mod->last_func != NULL) {
        mod->last_func->next = func;
    } else {
        mod->first_func = func;
    }

    mod->last_func = func;
    func->next = NULL;
    mod->func_count++;
}

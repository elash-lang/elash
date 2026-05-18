#include <elash/binder/binder.h>
#include <elash/util/assert.h>

ElHirModule* el_binder_bind_module(ElBinder* binder, ElAstModule* in) {
    ElHirModule* mod = el_hir_new_module(binder->hir_arena);
    for (ElAstTopLevel* node = in->head; node != NULL; node = node->next) {
        ElHirTopLevel* binded = el_binder_bind_toplvl(binder, node);
        if (binded == NULL) continue;

        el_hir_module_append(mod, binded);
    }
    mod->sym_count = binder->sym_id_counter;
    return mod;
}

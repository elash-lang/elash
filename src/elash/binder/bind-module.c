#include "binder-internals.h"
#include <elash/util/assert.h>

ElHirModule* el_bind_module(ElBinder* binder, ElAstModule* in) {
    EL_ASSERT(in != NULL, "should not be NULL");

    ElHirModule* mod = el_hir_new_module(binder->arena);
    for (ElAstDecl* node = in->head; node != NULL; node = node->next) {
        ElHirDecl* binded = el_bind_decl(binder, node);
        if (binded == NULL) continue;

        for (ElHirDecl* d = binded; d != NULL; d = d->next) {
            el_hir_module_append(mod, d);
        }
    }

    mod->sym_count = binder->sym_id_counter;
    return mod;
}

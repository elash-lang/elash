#include <elash/binder/binder.h>
#include <elash/util/assert.h>

ElHirTopLevel* el_binder_bind_toplvl(ElBinder* binder, ElAstTopLevel* in) {
    switch (in->type) {
    case EL_AST_TOPLVL_DECL: {
        ElHirDecl* decl = el_binder_bind_decl(binder, in->as.decl);
        if (decl == NULL) return NULL;
        return el_hir_new_toplevel_decl(binder->hir_arena, decl);
    }
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTopLevelType, in->type);
}

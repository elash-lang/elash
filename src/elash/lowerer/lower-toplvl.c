#include <elash/lowerer/lowerer.h>
#include <elash/util/assert.h>

void el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevel* hir) {
    switch (hir->kind) {
    case EL_HIR_TOPLVL_DECL:
        _el_lowerer_lower_global_decl(lw, hir->as.decl);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirTopLevelKind, hir->kind);
}

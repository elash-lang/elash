#include <elash/hir/tree/decl.h>

ElHirDecl* el_hir_new_var_def(ElDynArena* arena, ElHirSymbol* sym, ElHirExpr* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirDecl, {
        .kind = EL_HIR_DECL_VAR_DEF,
        .as.var_def = {
            .var = sym,
            .init = init,
        },
    });
}

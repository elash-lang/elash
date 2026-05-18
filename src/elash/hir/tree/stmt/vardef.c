#include <elash/hir/tree/stmt.h>

ElHirStmt* el_hir_new_var_def_stmt(ElDynArena* arena, ElSymbol* sym, ElHirExpr* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmt, {
        .kind = EL_HIR_STMT_VAR_DEF,
        .next = NULL,
        .as.var_def = {
            .var = sym,
            .init = init,
        },
    });
}

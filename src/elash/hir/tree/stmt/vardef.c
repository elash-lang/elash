#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_var_def_stmt(ElDynArena* arena, ElSymbol* sym, ElHirExprNode* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirStmtNode, {
        .kind = EL_HIR_STMT_VAR_DEF,
        .next = NULL,
        .as.var_def = {
            .var = sym,
            .init = init,
        },
    });
}

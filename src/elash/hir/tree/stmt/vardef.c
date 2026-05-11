#include <elash/hir/tree/stmt.h>

ElHirStmtNode* el_hir_new_var_def_stmt(ElDynArena* arena, ElSymbol* sym, ElHirExprNode* init) {
    ElHirStmtNode* node = EL_DYNARENA_NEW(arena, ElHirStmtNode);
    *node = (ElHirStmtNode) {
        .kind = EL_HIR_STMT_VAR_DEF,
        .next = NULL,
        .as.var_def = {
            .var = sym,
            .init = init,
        },
    };
    return node;
}

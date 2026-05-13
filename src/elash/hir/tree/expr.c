#include <elash/hir/tree/expr.h>

ElHirExprNode* el_hir_new_symbol_expr(ElDynArena* arena, ElType* type, ElSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExprNode, {
        .kind = EL_HIR_EXPR_SYMBOL,
        .type = type,
        .as.symbol = symbol,
    });
}

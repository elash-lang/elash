#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_symbol_expr(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElHirSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_SYMBOL,
        .type = type,
        .span = span,
        .as.symbol = symbol,
    });
}

#include <elash/hir/tree/expr.h>
#include <elash/util/assert.h>

ElHirExpr* el_hir_new_symbol_expr(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElHirSymbol* symbol) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_SYMBOL,
        .type = type,
        .span = span,
        .as.symbol = symbol,
    });
}

bool el_hir_expr_is_lvalue(const ElHirExpr* hir) {
    EL_ASSERT(hir != NULL, "should not be null");

    switch (hir->kind) {
    case EL_HIR_EXPR_SYMBOL:
        return hir->as.symbol->kind == EL_SYM_VAR || hir->as.symbol->kind == EL_SYM_FUNC;
    case EL_HIR_EXPR_UNARY:
        return hir->as.unary.op == EL_SEMA_UNARY_OP_DEREF || hir->as.unary.op == EL_SEMA_UNARY_OP_OPT_UNWRAP;
    case EL_HIR_EXPR_BINARY:
        return hir->as.binary.op == EL_SEMA_BIN_OP_INDEX;
    case EL_HIR_EXPR_MEMBER:
        return el_hir_expr_is_lvalue(hir->as.member.expr);
    case EL_HIR_EXPR_TMEMBER:
        return el_hir_expr_is_lvalue(hir->as.tmember.expr);
    case EL_HIR_EXPR_AGGINIT:
        return true;
    default:
        return false;
    }
}

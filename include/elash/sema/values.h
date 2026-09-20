#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>

#include <elash/hir/tree/expr.h>

static inline bool el_is_lvalue(ElHirExpr* operand) {
    return 0
        || operand->kind == EL_HIR_EXPR_SYMBOL
        || operand->kind == EL_HIR_EXPR_MEMBER
        || operand->kind == EL_HIR_EXPR_TMEMBER
        ||(operand->kind == EL_HIR_EXPR_BINARY && operand->as.binary.op == EL_BIN_OP_INDEX)
        ||(operand->kind == EL_HIR_EXPR_UNARY  && operand->as.unary.op == EL_UNARY_OP_DEREF);
}

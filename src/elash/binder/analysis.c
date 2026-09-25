#include "binder-internals.h"

#include <elash/util/assert.h>

bool _el_binder_stmt_always_returns(ElBinder* binder, ElHirStmt* stmt) {
    (void) binder;

    EL_ASSERT(stmt != NULL, "null statement passed to stmt-always-returns");
    switch (stmt->kind) {
    case EL_HIR_STMT_RETURN:
        return true;
    case EL_HIR_STMT_BLOCK:
        return _el_binder_block_always_returns(binder, stmt->as.block);

    case EL_HIR_STMT_IF:
        if (stmt->as.if_.else_ == NULL) return false;
        return _el_binder_stmt_always_returns(binder, stmt->as.if_.then)
            && _el_binder_stmt_always_returns(binder, stmt->as.if_.else_);
    case EL_HIR_STMT_WHILE:
        return _el_binder_stmt_always_returns(binder, stmt->as.while_.body);

    default:
        return false;
    }
}

bool _el_binder_block_always_returns(ElBinder* binder, ElHirBlockStmt block) {
    for (ElHirStmt* stmt = block.stmts; stmt != NULL; stmt = stmt->next) {
        if (_el_binder_stmt_always_returns(binder, stmt)) {
            return true;
        }
    }
    return false;
}

bool _el_binder_ensure_complete(ElBinder* binder, ElSourceSpan span, ElHirType* type) {
    if (type == NULL) return false;

    if (el_hir_type_is_incomplete(type)) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.incomplete-type",
            span, "invalid use of incomplete type '${type}'",
            EL_DIAG_TYPE("type", type),
        );
    }

    return true;
}

bool _el_binder_ensure_readable(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr) {
    if (expr == NULL || expr->type == NULL) return true;
    if (el_hir_type_mut(expr->type) != EL_MUTSPEC_WONLY) return true;
    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.write-only",
        span, "cannot read from write-only value of type '${type}'",
        EL_DIAG_TYPE("type", expr->type),
    );
}

bool _el_binder_ensure_writable(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr) {
    if (expr == NULL || expr->type == NULL) return true;
    if (el_hir_type_mut(expr->type) != EL_MUTSPEC_CONST) return true;
    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.immutable",
        span, "cannot modify immutable value of type '${type}'",
        EL_DIAG_TYPE("type", expr->type),
    );
}

// short circuit-like optimization
static inline Redundancy rcombine1(ElBinder* binder, ElHirExpr* expr) {
    if (_el_binder_redundancy_if_ignored(binder, expr) == REDUNDANCY_FULL) {
        return REDUNDANCY_FULL;
    }
    return REDUNDANCY_PARTIAL;
}
static inline Redundancy rcombine2(ElBinder* binder, ElHirExpr* a, ElHirExpr* b) {
    if (_el_binder_redundancy_if_ignored(binder, a) == REDUNDANCY_FULL &&
        _el_binder_redundancy_if_ignored(binder, b) == REDUNDANCY_FULL) {
        return REDUNDANCY_FULL;
    }
    return REDUNDANCY_PARTIAL;
}

Redundancy _el_binder_redundancy_if_ignored(ElBinder* binder, ElHirExpr* expr) {
    (void) binder;
    EL_ASSERT(expr != NULL, "should not be null");

    switch (expr->kind) {
    case EL_HIR_EXPR_CALL:
    case EL_HIR_EXPR_INTR:
        // TODO: check if the callee has pure attribute
        //       (when we will have attributes)
        return REDUNDANCY_NONE;

    case EL_HIR_EXPR_BINARY:
        return rcombine2(binder, expr->as.binary.left, expr->as.binary.right);
    case EL_HIR_EXPR_UNARY:
        if (el_unary_op_is_pre_incdec(expr->as.unary.op))
            return REDUNDANCY_NONE;
        else if (el_unary_op_is_post_incdec(expr->as.unary.op))
            return REDUNDANCY_PARTIAL;

        return rcombine1(binder, expr->as.unary.operand);

    case EL_HIR_EXPR_CAST:
        return rcombine1(binder, expr->as.cast.expr);
    case EL_HIR_EXPR_MEMBER:
        return rcombine1(binder, expr->as.member.expr);
    case EL_HIR_EXPR_TMEMBER:
        return rcombine1(binder, expr->as.tmember.expr);

    case EL_HIR_EXPR_AGGINIT:
        // we theoretically can iterate over elements and recurse
        // but i believe its not needed and will only slow down things
        return REDUNDANCY_PARTIAL;

    // list all cases explicitly instead of a default case
    // so the compiler will error out if new expression kinds were added
    // and are not handled here
    case EL_HIR_EXPR_STRCONST:
    case EL_HIR_EXPR_LITERAL:
    case EL_HIR_EXPR_CONST:
    case EL_HIR_EXPR_SYMBOL:
        return REDUNDANCY_FULL;
    }

    EL_UNREACHABLE_ENUM_VAL(ElHirExprKind, expr->kind);
}

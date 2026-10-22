#include "../binder-internals.h"

#include <elash/hir/tree/expr/intr.h>

ElHirExpr* _el_bind_exchange_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    if (!_el_binder_ensure_params(binder, in, 2, EL_SV("exchange")))
        return NULL;

    ElAstToI* dst_arg = call->args;
    ElAstToI* value_arg = call->args->next;
    EL_ASSERT(dst_arg != NULL && value_arg != NULL, "should not be null");

    if (dst_arg->kind == EL_AST_TOI_TYPE || value_arg->kind == EL_AST_TOI_TYPE) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            in->span, "arguments to 'exchange' must not be types"
        );
    }

    ElHirToE* dst_toe = _el_binder_ensure_toe(binder, dst_arg, EL_SV("exchange"));
    if (dst_toe == NULL) return NULL;

    if (dst_toe->is_type) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            dst_arg->span, "first argument to 'exchange' must be an expression"
        );
    }

    ElHirExpr* dst = dst_toe->as.expr;
    if (dst == NULL) return NULL;

    if (!_el_binder_is_lvalue(dst)) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.assign-rvalue",
            dst_arg->span, "cannot assign to rvalue",
        );
    }

    if (!_el_binder_ensure_writable(binder, dst_arg->span, dst))
        return NULL;
    if (!_el_binder_ensure_readable(binder, dst_arg->span, dst))
        return NULL;

    ElHirToE* value_toe = el_bind_toi(binder, value_arg, dst->type, EL_STORAGECLS_LOCAL);
    if (value_toe == NULL) return NULL;

    if (value_toe->is_type) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            value_arg->span, "second argument to 'exchange' must be an expression or initializer"
        );
    }

    ElHirExpr* value = value_toe->as.expr;
    if (value == NULL) return NULL;

    return el_hir_new_exchange_intr(binder->arena, in->span, dst, value);
}

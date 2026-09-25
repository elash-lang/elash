#include "../binder-internals.h"

static ElHirExpr* len_from_array_type(ElBinder* binder, ElSourceSpan span, ElHirType* type) {
    if (type == NULL) return NULL;
    type = el_hir_type_unwrap(type);
    if (type->kind != EL_HIR_TYPE_ARRAY) return NULL;

    return el_hir_new_int_constant(
        binder->arena, span, binder->builtins->type_usize, EL_INT128((int64_t)type->as.array.size)
    );
}

ElHirExpr* _el_bind_len_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    if (!_el_binder_ensure_params(binder, in, 1, EL_SV("len")))
        return NULL;

    ElAstToI* arg = call->args;
    EL_ASSERT(arg != NULL, "should not be null");

    ElHirToE* toe = _el_binder_ensure_toe(binder, arg, EL_SV("len"));
    if (toe == NULL) return NULL;

    if (toe->is_type) {
        ElHirExpr* len = len_from_array_type(binder, in->span, toe->as.type);
        if (len != NULL) return len;

        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            arg->span, "argument to 'len' must be an array or slice type"
        );
    }

    ElHirExpr* earg = toe->as.expr;
    if (earg == NULL) return NULL;
    earg = _el_binder_apply_default_type(binder, earg);

    ElHirType* type = el_hir_type_unwrap(earg->type);

    if (type->kind == EL_HIR_TYPE_ARRAY) {
        return el_hir_new_int_constant(
            binder->arena, in->span, binder->builtins->type_usize,
            EL_INT128((int64_t)type->as.array.size)
        );
    }
    if (type->kind == EL_HIR_TYPE_SLICE) {
        return el_hir_new_slice_len_intr(binder->arena, in->span, binder->builtins->type_usize, earg);
    }

    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
        arg->span, "argument to 'len' must be an array or slice"
    );
}

ElHirExpr* _el_bind_mkslice_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    if (!_el_binder_ensure_params(binder, in, 2, EL_SV("mkslice")))
        return NULL;

    ElAstToI* raw_arg = call->args;
    ElAstToI* len_arg = call->args->next;
    EL_ASSERT(raw_arg != NULL && len_arg != NULL, "should not be null");

    ElHirToE* raw_toe = _el_binder_ensure_toe(binder, raw_arg, EL_SV("mkslice"));
    ElHirToE* len_toe = _el_binder_ensure_toe(binder, len_arg, EL_SV("mkslice"));
    if (raw_toe == NULL || len_toe == NULL) return NULL;

    if (raw_toe->is_type || len_toe->is_type) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            in->span,
            "expressions were expected as arguments to the mkslice function"
        );
        return NULL;
    }

    ElHirExpr* raw = raw_toe->as.expr;
    if (raw == NULL) return NULL;
    ElHirType* raw_ty = el_hir_type_canonical(raw->type);
    if (raw_ty == NULL || raw_ty->kind != EL_HIR_TYPE_RWSLICE) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            raw_arg->span, "first argument to 'mkslice' must be a raw slice"
        );
    }

    ElHirExpr* len = len_toe->as.expr;
    if (len == NULL) return NULL;

    len = _el_binder_implicit_cast(binder, len_arg->span, len, binder->builtins->type_usize);
    if (len == NULL) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            len_arg->span, "second argument to 'mkslice' must be convertible to usize"
        );
    }

    return el_hir_new_make_slice_intr(binder->arena, in->span, raw, len);
}


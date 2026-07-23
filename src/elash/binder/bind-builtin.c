#include <elash/binder/binder.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

ElHirExpr* _el_binder_bind_len_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    if (call->arg_count != 1) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
            in->span, "expected 1 argument for 'len', but got ${got}",
            EL_DIAG_INT("got", call->arg_count)
        );
    }

    if (call->args[0].kind != EL_AST_INIT_EXPR) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            in->span, "an expression was expected as an argument to the len function",
        );
    }

    ElHirExpr* arg = el_binder_bind_expr(binder, call->args[0].expr);
    if (!arg) return NULL;

    if (arg->type->kind == EL_HIR_TYPE_ARRAY) {
        return el_hir_new_int_constant(binder->hir_arena, in->span, binder->builtins->type_usize, (int64_t)arg->type->as.array.size);
    }
    if (arg->type->kind == EL_HIR_TYPE_SLICE) {
        return el_hir_new_slice_len_intr(binder->hir_arena, in->span, binder->builtins->type_usize, arg);
    }

    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
        call->args->span,"argument to 'len' must be an array or slice"
    );
}

ElHirExpr* _el_binder_bind_mkslice_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    if (call->arg_count != 2) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
            in->span, "expected 2 arguments for 'mkslice', but got ${got}",
            EL_DIAG_INT("got", call->arg_count)
        );
    }

    ElAstInit* raw_arg = call->args;
    ElAstInit* len_arg = call->args->next;

    if (raw_arg->kind != EL_AST_INIT_EXPR || len_arg->kind != EL_AST_INIT_EXPR) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            in->span,
            "expressions were expected as arguments to the mkslice function",
        );
        return NULL;
    }

    ElHirExpr* raw = el_binder_bind_expr(binder, raw_arg->expr);
    if (raw->type->kind != EL_HIR_TYPE_RWSLICE) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            raw_arg->span, "first argument to 'mkslice' must be a raw slice"
        );
    }

    ElHirExpr* len = el_binder_bind_expr(binder, len_arg->expr);
    if (len == NULL) return NULL;

    len = _el_binder_implicit_cast(binder, len_arg->span, len, binder->builtins->type_usize);
    if (len == NULL) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            len_arg->span, "second argument to 'mkslice' must be convertible to usize"
        );
    }

    return el_hir_new_make_slice_intr(binder->hir_arena, in->span, raw, len);
}

ElHirExpr* el_binder_bind_builtin_call(
    ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call, ElHirSymbol* builtin
) {
    switch (builtin->as.builtin.kind) {
    case EL_BUILTIN_LEN:
        return _el_binder_bind_len_call(binder, in, call);
    case EL_BUILTIN_MKSLICE:
        return _el_binder_bind_mkslice_call(binder, in, call);
    }
    EL_UNREACHABLE_ENUM_VAL(ElBuiltinKind, builtin->as.builtin.kind);
}

#include <elash/binder/binder.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

ElHirExpr* _el_binder_bind_len_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    if (call->arg_count != 1) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
            in->span,
            "expected 1 argument for 'len', but got ${got}",
            EL_DIAG_INT("got", call->arg_count)
        );
        return NULL;
    }

    ElHirExpr* arg = el_binder_bind_expr(binder, call->args);
    if (!arg) return NULL;

    if (arg->type->kind == EL_TYPE_ARRAY) {
        return el_hir_new_int_constant(binder->hir_arena, binder->builtins->type_int, (int64_t)arg->type->as.array.size);
    }
    if (arg->type->kind == EL_TYPE_SLICE) {
        EL_TODO("implement len for slices");
    }

    el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
        call->args->span,
        "argument to 'len' must be an array or slice"
    );
    return NULL;
}

ElHirExpr* el_binder_bind_builtin_call(
    ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call, ElSymbol* builtin
) {
    switch (builtin->as.builtin.kind) {
    case EL_BUILTIN_LEN:
        return _el_binder_bind_len_call(binder, in, call);
    }
    EL_UNREACHABLE_ENUM_VAL(ElBuiltinKind, builtin->as.builtin.kind);
}

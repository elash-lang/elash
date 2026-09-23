#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>

ElHirToE* _el_binder_ensure_toe(ElBinder* binder, ElAstToI* toi, ElStringView bname) {
    if (toi->kind == EL_AST_TOI_INIT && toi->as.init->kind != EL_AST_INIT_EXPR) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-builtin-call",
            toi->span, "argument to '${bname}' must be an expression or a type",
            EL_DIAG_STRING("bname", bname),
        );
        el_diag_help(
            binder->diag, "non-expression initializers cannot be passed directly to '${bname}'",
            EL_DIAG_STRING("bname", bname),
        );
        return NULL;
    }

    return el_bind_toi(binder, toi, NULL, EL_STORAGECLS_LOCAL);
}

bool _el_binder_ensure_params(ElBinder* binder, ElAstExpr* in, usize count, ElStringView bname) {
    if (in->as.call.arg_count != count) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
            in->span, "expected ${count} argument${s} for '${bname}', but got ${got}",
            EL_DIAG_INT("count", count),
            EL_DIAG_INT("got", in->as.call.arg_count),
            EL_DIAG_STRING("s", count > 1 ? EL_SV("s") : EL_SV_NULL),
            EL_DIAG_STRING("bname", bname),
        );
    }

    return true;
}

ElHirExpr* el_bind_builtin_call(
    ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call, ElHirSymbol* builtin
) {
    switch (builtin->as.builtin.kind) {
    case EL_BUILTIN_LEN:
        return _el_bind_len_call(binder, in, call);
    case EL_BUILTIN_MKSLICE:
        return _el_bind_mkslice_call(binder, in, call);
    case EL_BUILTIN_SIZEOF:
        return _el_bind_sizeof(binder, in, call);
    case EL_BUILTIN_ALIGNOF:
        return _el_bind_alignof(binder, in, call);
    }
    EL_UNREACHABLE_ENUM_VAL(ElBuiltinKind, builtin->as.builtin.kind);
}

#include "../binder-internals.h"

#include <elash/sema/backends.h>
#include <elash/sema/tcache.h>

static ElBSType* process(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call, ElStringView bname) {
    if (!_el_binder_ensure_params(binder, in, 1, bname))
        return NULL;

    ElAstToI* arg = call->args;
    EL_ASSERT(arg != NULL, "should not be null");

    ElHirToE* toe = _el_binder_ensure_toe(binder, arg, bname);
    if (toe == NULL) return NULL;

    ElHirType* ttype = toe->is_type
        ? toe->as.type
        : toe->as.expr->type;

    if (el_hir_type_is_incomplete(ttype)) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.incomplete-type",
            arg->span, "cannot pass incomplete type '${type}' to builtin '${bname}' function",
            EL_DIAG_TYPE("type", ttype),
            EL_DIAG_STRING("bname", bname),
        );
    }

    return el_tcache_get_bst_from_hir(binder->tcache, ttype);
}

ElHirExpr* _el_bind_sizeof(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    ElBSType* bstype = process(binder, in, call, EL_SV("sizeof"));
    if (bstype == NULL) return NULL;

    usize size = binder->bsquery->get_size(binder->bsquery, bstype);
    return el_hir_new_int_lit(binder->arena, in->span, EL_INT128(size));
}

ElHirExpr* _el_bind_alignof(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    ElBSType* bstype = process(binder, in, call, EL_SV("alignof"));
    if (bstype == NULL) return NULL;

    usize alignment = binder->bsquery->get_align(binder->bsquery, bstype);
    return el_hir_new_int_lit(binder->arena, in->span, EL_INT128(alignment));
}


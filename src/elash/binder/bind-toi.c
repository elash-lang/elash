#include "binder-internals.h"
#include <elash/ast/tree/toi.h>
#include <elash/hir/toe.h>
#include <elash/diag/engine.h>

static ElHirToE* _bind_toi_internal(ElBinder* binder, ElAstToI* in, ElHirType* expected_type, ElStorageClass scls) {
    ElHirToE* toe = NULL;
    switch (in->kind) {
    case EL_AST_TOI_TYPE: {
        ElHirType* type = el_bind_type(binder, in->as.type);
        if (type == NULL) return NULL;
        return el_hir_new_toe_type(binder->arena, type);
    }
    case EL_AST_TOI_INIT: {
        if (expected_type == NULL && in->as.init->kind == EL_AST_INIT_EXPR) {
            ElHirExpr* expr = el_bind_expr(binder, in->as.init->expr);
            if (expr == NULL) return NULL;
            toe = el_hir_new_toe_expr(binder->arena, expr);
        } else {
            ElHirExpr* expr = el_bind_init(binder, in->as.init, expected_type, scls);
            if (expr == NULL) return NULL;
            return el_hir_new_toe_expr(binder->arena, expr);
        }
        break;
    }
    case EL_AST_TOI_UNR:
        toe = _el_bind_unresolved(binder, el_ast_new_toe_unr(binder->arena, in->as.unr), in->as.unr);
        break;
    }

    if (toe == NULL) return NULL;

    if (!toe->is_type && expected_type != NULL) {
        ElHirExpr* casted = _el_binder_implicit_cast(binder, in->span, toe->as.expr, expected_type);
        if (casted == NULL) return NULL;
        toe->as.expr = _el_binder_simplify_expr(binder, casted);
    }

    return toe;
}


ElHirToE* el_bind_toi(ElBinder* binder, ElAstToI* in, ElHirType* expected_type, ElStorageClass scls) {
    EL_ASSERT(in != NULL, "should not be NULL");
    el_prof_begin_sub(binder->prof, binder->pss_toi);
    ElHirToE* result = _bind_toi_internal(binder, in, expected_type, scls);
    el_prof_finish_sub(binder->prof, binder->pss_toi);
    return result;
}

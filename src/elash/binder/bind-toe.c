#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/hir/toe.h>

static ElHirToE* _bind_toe_internal(ElBinder* binder, ElAstToE* in) {
    switch (in->kind) {
    case EL_AST_TOE_TYPE: {
        ElHirType* type = el_bind_type(binder, in->as.type);
        if (type == NULL) return NULL;
        return el_hir_new_toe_type(binder->arena, type);
    }
    case EL_AST_TOE_EXPR: {
        ElHirExpr* expr = el_bind_expr(binder, in->as.expr);
        if (expr == NULL) return NULL;
        return el_hir_new_toe_expr(binder->arena, expr);
    }
    case EL_AST_TOE_UNR:
        return _el_bind_unresolved(binder, in, in->as.unr);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, in->kind);
}

ElHirToE* el_bind_toe(ElBinder* binder, ElAstToE* in) {
    EL_ASSERT(in != NULL, "should not be NULL");
    el_prof_begin_sub(binder->prof, binder->pss_toe);
    ElHirToE* result = _bind_toe_internal(binder, in);
    el_prof_finish_sub(binder->prof, binder->pss_toe);
    return result;
}

#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/toe.h>

static ElHirToE* resolve_toe_by_kind(ElBinder* binder, ElAstToE* in, bool is_type) {
    if (is_type) {
        ElAstType* ast_type = el_ast_toe_as_type(binder->hir_arena, in);
        ElHirType* hir_type = _el_binder_bind_type(binder, ast_type);
        if (hir_type == NULL) return NULL;
        return el_hir_new_toe_type(binder->hir_arena, hir_type);
    } else {
        ElAstExpr* ast_expr = el_ast_toe_as_expr(binder->hir_arena, in);
        ElHirExpr* hir_expr = el_binder_bind_expr(binder, ast_expr);
        if (hir_expr == NULL) return NULL;
        return el_hir_new_toe_expr(binder->hir_arena, hir_expr);
    }
}

ElHirToE* el_binder_bind_toe(ElBinder* binder, ElAstToE* in) {
    if (in == NULL) return NULL;

    switch (in->kind) {
    // easy cases
    case EL_AST_TOE_TYPE: {
        ElHirType* type = _el_binder_bind_type(binder, in->as.type);
        if (type == NULL) return NULL;
        return el_hir_new_toe_type(binder->hir_arena, type);
    }
    case EL_AST_TOE_EXPR: {
        ElHirExpr* expr = el_binder_bind_expr(binder, in->as.expr);
        if (expr == NULL) return NULL;
        return el_hir_new_toe_expr(binder->hir_arena, expr);
    }

    // very very advanced cases
    case EL_AST_TOE_UNR_IDENT: {
        ElHirSymbol* sym = el_hir_scope_lookup(binder->current_scope, in->as.unr_ident->name);
        if (sym == NULL) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unresolved-symbol",
                in->span, "unresolved symbol: ${name}",
                EL_DIAG_STRING("name", in->as.unr_ident->name)
            );
        }

        return resolve_toe_by_kind(binder, in, sym->kind == EL_SYM_TYPE);
    }
    case EL_AST_TOE_UNR_INDEX: {
        ElHirToE* base_toe = el_binder_bind_toe(binder, in->as.unr_index.base);
        if (base_toe == NULL) return NULL;

        return resolve_toe_by_kind(binder, in, base_toe->is_type);
    }
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, in->kind);
}

#include "binder-internals.h"

static ElHirToE* resolve_toe_by_kind(ElBinder* binder, ElAstUnr* in, bool is_type) {
    if (is_type) {
        ElAstType* ast_type = el_ast_unr_as_type(binder->arena, in);
        ElHirType* hir_type = el_bind_type(binder, ast_type);
        if (hir_type == NULL) return NULL;
        return el_hir_new_toe_type(binder->arena, hir_type);
    } else {
        ElAstExpr* ast_expr = el_ast_unr_as_expr(binder->arena, in);
        ElHirExpr* hir_expr = el_bind_expr(binder, ast_expr);
        if (hir_expr == NULL) return NULL;
        return el_hir_new_toe_expr(binder->arena, hir_expr);
    }
}

ElHirToE* _el_bind_unresolved(ElBinder* binder, ElAstToE* in, ElAstUnr* unr) {
    if (unr->kind == EL_AST_UNR_IDENT) {
        ElHirSymbol* sym = el_hir_scope_lookup(binder->current_scope, unr->as.ident->name);
        if (sym == NULL) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unresolved-symbol",
                in->span, "unresolved symbol: ${name}",
                EL_DIAG_STRING("name", unr->as.ident->name)
            );
        }

        return resolve_toe_by_kind(binder, unr, sym->kind == EL_SYM_TYPE);
    }

    ElHirToE* base_toe = el_bind_toe(binder, el_ast_new_toe_unr(binder->arena, unr->as.index.base));
    if (base_toe == NULL) return NULL;

    return resolve_toe_by_kind(binder, unr, base_toe->is_type);
}

#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/type/func.h>
#include <elash/hir/tree/decl.h>

#define REPORT_PARAM_REDECLARATION(BINDER, PARAM)            \
    el_diag_report(                                          \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.redeclaration", \
        (PARAM)->span,                                       \
        "redeclaration of parameter '${name}'",              \
        EL_DIAG_STRING("name", (PARAM)->name->name)          \
    );

static bool _el_binder_bind_param_types(ElBinder* binder, ElAstFuncParamList* params,
                              ElHirType*** out_param_types, usize* out_count) {
    usize count = params->count;
    ElHirType** param_types = EL_DYNARENA_NEW_ARR(binder->type_arena, ElHirType*, count);
    bool has_error = false;
    usize i = 0;

    for (ElAstFuncParam* param = params->head; param != NULL; param = param->next) {
        param_types[i] = _el_binder_bind_type(binder, param->type);
        if (param_types[i] == NULL) has_error = true;
        i++;
    }

    if (has_error) return false;

    *out_param_types = param_types;
    *out_count = count;
    return true;
}

static bool _el_binder_create_param_symbols(
    ElBinder* binder, ElAstFuncParamList* params,
    ElHirType** param_types, ElHirSymbol*** out_params
) {
    usize count = params->count;
    ElHirSymbol** param_syms = EL_DYNARENA_NEW_ARR(binder->sym_arena, ElHirSymbol*, count);
    bool has_error = false;
    usize i = 0;

    for (ElAstFuncParam* param = params->head; param != NULL; param = param->next) {
        for (usize j = 0; j < i; j++) {
            if (el_sv_eql(param_syms[j]->name, param->name->name)) {
                REPORT_PARAM_REDECLARATION(binder, param);
                has_error = true;
                break;
            }
        }

        param_syms[i] = el_hir_new_var_symbol(
            binder->sym_arena, binder->sym_id_counter++,
            param->name->name, param_types[i]
        );
        i++;
    }

    if (has_error) return false;

    *out_params = param_syms;
    return true;
}

ElHirSymbol* _el_binder_bind_func_sig(ElBinder* binder, ElAstFuncSignature* sig) {
    ElHirType* ret_type = _el_binder_bind_type(binder, sig->ret_type);

    ElHirType** param_types = NULL;
    usize param_count = 0;
    if (!_el_binder_bind_param_types(binder, &sig->params, &param_types, &param_count))
        return NULL;

    ElHirType* func_type = el_hir_new_func_type(
        binder->type_arena, ret_type, param_types, param_count
    );

    ElHirSymbol* existing = el_hir_scope_lookup_local(binder->global_scope, sig->name->name);
    if (existing) {
        if (existing->kind != EL_SYM_FUNC) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-redeclaration",
                sig->name->span,
                "redeclaration of symbol '${name}' as a different kind of symbol",
                EL_DIAG_STRING("name", sig->name->name)
            );
            return NULL;
        }

        if (!el_hir_type_eql(existing->as.func.type, func_type)) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.conflicting-types",
                sig->name->span,
                "conflicting types for '${name}'",
                EL_DIAG_STRING("name", sig->name->name)
            );
            return NULL;
        }
        return existing; // if we get here, then function is already declared with the same signature
    }

    ElHirSymbol** param_syms = NULL;
    if (!_el_binder_create_param_symbols(binder, &sig->params, param_types, &param_syms))
        return NULL;

    ElHirSymbol* sym = el_hir_new_func_symbol(
        binder->sym_arena, binder->sym_id_counter++, sig->name->name,
        ret_type, param_syms, param_count
    );
    (void) el_hir_scope_insert(binder->global_scope, sym);
    return sym;
}

static ElHirDecl* _el_binder_bind_var_def(ElBinder* binder, ElAstDecl* in, ElAstVarDef* var) {
    ElHirType* type = _el_binder_bind_type(binder, var->type);
    if (!type) return NULL;

    if (type->kind == EL_HIR_TYPE_PRIM && type->as.prim.kind == EL_PRIMTYPE_VOID) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.incomplete-type",
            var->type->span,
            "cannot declare variable of incomplete type 'void'"
        );
        return NULL;
    }

    ElHirSymbol* sym = el_hir_new_var_symbol(binder->sym_arena, binder->sym_id_counter++, var->name->name, type);
    if (!el_hir_scope_insert(binder->current_scope, sym)) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.redeclaration",
            var->name->span,
            "redeclaration of symbol '${name}'",
            EL_DIAG_STRING("name", sym->name)
        );
        return NULL;
    }

    ElHirExpr* init = NULL;
    if (var->init != NULL) {
        init = el_binder_bind_init(binder, var->init, type);
        if (!init) return NULL;
    }

    return el_hir_new_var_def(binder->hir_arena, in->span, sym, init);
}

static ElHirDecl* _el_binder_bind_var_decl(ElBinder* binder, ElAstDecl* in, ElAstVarDecl* var) {
    ElHirType* type = _el_binder_bind_type(binder, var->type);
    if (!type) return NULL;

    if (type->kind == EL_HIR_TYPE_PRIM && type->as.prim.kind == EL_PRIMTYPE_VOID) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.incomplete-type",
            var->type->span,
            "cannot declare variable of incomplete type 'void'"
        );
        return NULL;
    }

    ElHirSymbol* sym = el_hir_new_var_symbol(binder->sym_arena, binder->sym_id_counter++, var->name->name, type);
    if (!el_hir_scope_insert(binder->current_scope, sym)) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.redeclaration",
            var->name->span,
            "redeclaration of symbol '${name}'",
            EL_DIAG_STRING("name", sym->name)
        );
        return NULL;
    }

    return el_hir_new_var_decl(binder->hir_arena, in->span, sym);
}

static ElHirDecl* _el_binder_bind_func_def(ElBinder* binder, ElAstDecl* in, ElAstFuncDef* def) {
    ElHirSymbol* sym = _el_binder_bind_func_sig(binder, &def->sig);
    if (sym == NULL) return NULL;

    if (sym->as.func.is_defined) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.redefinition",
            def->sig.name->span,
            "redefinition of function '${name}'",
            EL_DIAG_STRING("name", def->sig.name->name)
        );
        return NULL;
    }
    sym->as.func.is_defined = true;

    ElHirSymbol* prev_func = binder->current_func;
    binder->current_func = sym;

    _el_binder_push_scope(binder);
    for (usize i = 0; i < sym->as.func.param_count; ++i) {
        (void) el_hir_scope_insert(binder->current_scope, sym->as.func.params[i]);
    }

    ElHirBlockStmt block = _el_binder_bind_block(binder, def->block);
    _el_binder_pop_scope(binder);

    binder->current_func = prev_func;

    return el_hir_new_func_def(binder->hir_arena, in->span, sym, block);
}

static ElHirDecl* _el_binder_bind_func_decl(ElBinder* binder, ElAstDecl* in, ElAstFuncDecl* decl) {
    ElHirSymbol* sym = _el_binder_bind_func_sig(binder, &decl->sig);
    if (sym == NULL) return NULL;
    return el_hir_new_func_decl(binder->hir_arena, in->span, sym);
}

ElHirDecl* el_binder_bind_decl(ElBinder* binder, ElAstDecl* in) {
    switch (in->type) {
    case EL_AST_DECL_VAR_DEF:
        return _el_binder_bind_var_def(binder, in, &in->as.var_def);
    case EL_AST_DECL_VAR_DECL:
        return _el_binder_bind_var_decl(binder, in, &in->as.var_decl);
    case EL_AST_DECL_FUNC_DEF:
        return _el_binder_bind_func_def(binder, in, &in->as.func_def);
    case EL_AST_DECL_FUNC_DECL:
        return _el_binder_bind_func_decl(binder, in, &in->as.func_decl);
    case EL_AST_DECL_ALIAS:
        EL_TODO("support aliases");
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDeclType, in->type);
}

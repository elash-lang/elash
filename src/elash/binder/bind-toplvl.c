#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type/func.h>

#define REPORT_PARAM_REDECLARATION(BINDER, PARAM)            \
    el_diag_report(                                          \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.redeclaration", \
        (PARAM)->span,                                       \
        "redeclaration of parameter '${name}'",              \
        EL_DIAG_STRING("name", (PARAM)->name->name)          \
    );

static bool _el_binder_bind_param_types(ElBinder* binder, ElAstFuncParamList* params,
                              ElType*** out_param_types, usize* out_count) {
    usize count = params->count;
    ElType** param_types = EL_DYNARENA_NEW_ARR(binder->arena, ElType*, count);
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
    ElType** param_types, ElSymbol*** out_params
) {
    usize count = params->count;
    ElSymbol** param_syms = EL_DYNARENA_NEW_ARR(binder->arena, ElSymbol*, count);
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

        param_syms[i] = el_sema_new_var_symbol(
            binder->arena, binder->sym_id_counter++,
            param->name->name, param_types[i]
        );
        i++;
    }

    if (has_error) return false;

    *out_params = param_syms;
    return true;
}

ElSymbol* _el_binder_bind_func_sig(ElBinder* binder, ElAstFuncSignature* sig) {
    ElType* ret_type = _el_binder_bind_type(binder, sig->ret_type);

    ElType** param_types = NULL;
    usize param_count = 0;
    if (!_el_binder_bind_param_types(binder, &sig->params, &param_types, &param_count))
        return NULL;
    
    ElType* func_type = el_sema_new_func_type(
        binder->arena, ret_type, param_types, param_count
    );

    ElSymbol* existing = el_sema_scope_lookup_local(binder->global_scope, sig->name->name);
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

        if (!el_sema_type_eql(existing->as.func.type, func_type)) {
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

    ElSymbol** param_syms = NULL;
    if (!_el_binder_create_param_symbols(binder, &sig->params, param_types, &param_syms))
        return NULL;

    ElSymbol* sym = el_sema_new_func_symbol(
        binder->arena, binder->sym_id_counter++, sig->name->name,
        ret_type, param_syms, param_count
    );
    (void) el_sema_scope_insert(binder->global_scope, sym);
    return sym;
}

ElHirTopLevelNode* _el_binder_bind_func_def(ElBinder* binder, ElAstFuncDef* def) {
    ElSymbol* sym = _el_binder_bind_func_sig(binder, &def->sig);
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

    _el_binder_push_scope(binder);
    bool has_error = false;
    for (ElAstFuncParam* param = def->sig.params.head; param != NULL; param = param->next) {
        ElType* type = _el_binder_bind_type(binder, param->type);
        ElSymbol* psym = el_sema_new_var_symbol(
            binder->arena, binder->sym_id_counter++,
            param->name->name, type
        );
        if (!el_sema_scope_insert(binder->current_scope, psym)) {
            REPORT_PARAM_REDECLARATION(binder, param);
            has_error = true;
        }
    }

    if (has_error) {
        _el_binder_pop_scope(binder);
        return NULL;
    }

    ElHirBlockStmtNode block = _el_binder_bind_block(binder, def->block);
    _el_binder_pop_scope(binder);

    return el_hir_new_func_def(binder->arena, sym, block);
}

ElHirTopLevelNode* el_binder_bind_toplvl(ElBinder* binder, ElAstTopLevelNode* in) {
    switch (in->type) {
    case EL_AST_TOPLVL_FUNC_DECL: {
        ElSymbol* sym = _el_binder_bind_func_sig(binder, &in->as.func_decl.sig);
        if (sym == NULL) return NULL;
        return el_hir_new_func_decl(binder->arena, sym);
    }
    case EL_AST_TOPLVL_FUNC_DEF:
        return _el_binder_bind_func_def(binder, &in->as.func_def);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTopLevelType, in->type);
}

#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/hir/type/func.h>
#include <elash/hir/tree/decl.h>

#define REPORT_PARAM_REDECLARATION(BINDER, PARAM)            \
    el_diag_report(                                          \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.redeclaration", \
        (PARAM)->span,                                       \
        "redeclaration of parameter '${name}'",              \
        EL_DIAG_STRING("name", (PARAM)->name->name)          \
    )

#define REPORT_REDEFINITION(BINDER, SPAN, NAME)              \
    el_diag_report(                                          \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.redefinition",  \
        (SPAN), "redefinition of symbol '${name}'",          \
        EL_DIAG_STRING("name", NAME)                         \
    )

static bool bind_param_types(
    ElBinder* binder, ElAstFuncParamList* params,
    ElHirType*** out_param_types, usize* out_count
) {
    usize count = params->count;
    ElHirType** param_types = EL_DYNARENA_NEW_ARR(binder->arena, ElHirType*, count);
    bool has_error = false;
    usize i = 0;

    for (ElAstFuncParam* param = params->head; param != NULL; param = param->next) {
        param_types[i] = el_bind_type(binder, param->type);
        if (!_el_binder_ensure_complete(binder, param->span, param_types[i]))
            has_error = true;

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
    ElHirSymbol** param_syms = EL_DYNARENA_NEW_ARR(binder->arena, ElHirSymbol*, count);

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
            binder->arena, binder->sym_id_counter++,
            param->name->name, param_types[i]
        );
        i++;
    }

    if (has_error) return false;

    *out_params = param_syms;
    return true;
}

static ElHirSymbol* bind_func_sig(ElBinder* binder, ElAstFuncSignature* sig) {
    ElHirType* ret_type = el_bind_type(binder, sig->ret_type);
    if (ret_type == NULL) return NULL;

    if (!el_hir_type_eql(ret_type, binder->builtins->type_void))
        if (!_el_binder_ensure_complete(binder, sig->ret_type->span, ret_type))
            return NULL;

    ElHirType** param_types = NULL;
    usize param_count = 0;
    if (!bind_param_types(binder, &sig->params, &param_types, &param_count))
        return NULL;

    ElHirType* func_type = el_hir_new_func_type(
        binder->arena, ret_type, param_types, param_count
    );

    ElHirSymbol* existing = el_hir_scope_lookup_local(binder->global_scope, sig->name->name);
    if (existing != NULL) {
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
        binder->arena, binder->sym_id_counter++, sig->name->name,
        ret_type, param_syms, param_count
    );
    (void) el_hir_scope_insert(binder->global_scope, sym);
    return sym;
}

static ElHirSymbol* bind_var_declarator_sym(
    ElBinder* binder, ElAstDeclarator* d, ElHirType* type, bool is_def
) {
    ElHirSymbol* sym = el_hir_new_var_symbol(binder->arena, binder->sym_id_counter++, d->name->name, type);
    if (!el_hir_scope_insert(binder->current_scope, sym)) {
        if (is_def) {
            REPORT_REDEFINITION(binder, d->name->span, sym->name);
        } else {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.redeclaration",
                d->name->span,
                "redeclaration of symbol '${name}'",
                EL_DIAG_STRING("name", sym->name)
            );
        }
        return NULL;
    }
    return sym;
}

static ElHirDecl* bind_var_def(ElBinder* binder, ElAstDecl* in, ElAstVarDef* var) {
    ElHirDecl* head = NULL;
    ElHirDecl* tail = NULL;

    ElHirType* type = el_bind_type(binder, var->type);
    if (!_el_binder_ensure_complete(binder, var->type->span, type))
        return NULL;

    for (ElAstDeclarator* d = var->declarators; d != NULL; d = d->next) {
        ElHirSymbol* sym = bind_var_declarator_sym(binder, d, type, true);
        if (sym == NULL) return NULL;

        ElStorageClass scls = (var->is_static || binder->current_func == NULL)
            ? EL_STORAGECLS_STATIC
            : EL_STORAGECLS_LOCAL;

        ElHirExpr* init = NULL;
        if (d->init != NULL) {
            init = el_bind_init(binder, d->init, type, scls);
            if (init == NULL) return NULL;
        }

        el_hir_append_decl(&head, &tail,
            el_hir_new_var_def(binder->arena, in->span, sym, init, scls));
    }

    return head;
}

static ElHirDecl* bind_var_decl(ElBinder* binder, ElAstDecl* in, ElAstVarDecl* var) {
    ElHirDecl* head = NULL;
    ElHirDecl* tail = NULL;

    ElHirType* type = el_bind_type(binder, var->type);
    if (!_el_binder_ensure_complete(binder, var->type->span, type))
        return NULL;

    for (ElAstDeclarator* d = var->declarators; d != NULL; d = d->next) {
        ElHirSymbol* sym = bind_var_declarator_sym(binder, d, type, false);
        if (sym == NULL) return NULL;

        el_hir_append_decl(&head, &tail,
            el_hir_new_var_decl(binder->arena, in->span, sym));
    }

    return head;
}

static ElHirDecl* bind_func_def(ElBinder* binder, ElAstDecl* in, ElAstFuncDef* def) {
    ElHirSymbol* sym = bind_func_sig(binder, &def->sig);
    if (sym == NULL) return NULL;

    if (sym->as.func.is_defined) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.redefinition",
            def->sig.name->span,
            "redefinition of function '${name}'",
            EL_DIAG_STRING("name", def->sig.name->name)
        );
    }
    sym->as.func.is_defined = true;

    ElHirSymbol* prev_func = binder->current_func;
    binder->current_func = sym;

    _el_binder_push_scope(binder);
    for (usize i = 0; i < sym->as.func.param_count; ++i) {
        (void) el_hir_scope_insert(binder->current_scope, sym->as.func.params[i]);
    }

    ElHirBlockStmt block = _el_bind_block(binder, def->block);
    _el_binder_pop_scope(binder);

    binder->current_func = prev_func;

    if (!el_hir_type_eql(sym->as.func.type->as.func.ret_type, binder->builtins->type_void)) {
        if (!_el_binder_block_always_returns(binder, block)) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.return-missing",
                def->sig.ret_type->span,
                "non-void function '${fn}' does not return a value in all control paths",
                EL_DIAG_STRING("fn", def->sig.name->name),
            );
        }
    }

    return el_hir_new_func_def(binder->arena, in->span, sym, block);
}

static ElHirDecl* bind_func_decl(ElBinder* binder, ElAstDecl* in, ElAstFuncDecl* decl) {
    ElHirSymbol* sym = bind_func_sig(binder, &decl->sig);
    if (sym == NULL) return NULL;
    return el_hir_new_func_decl(binder->arena, in->span, sym);
}

static ElHirDecl* bind_alias(ElBinder* binder, ElAstDecl* in, ElAstAlias* alias) {
    ElHirToE* toe = el_bind_toe(binder, &alias->target);
    if (toe == NULL) return NULL;

    if (toe->is_type) {
        ElHirSymbol* sym = el_hir_new_type_symbol(
            binder->arena, binder->sym_id_counter++, alias->name, toe->as.type);

        if (!el_hir_scope_insert(binder->current_scope, sym)) {
            return REPORT_REDEFINITION(binder, in->span, sym->name);
        }
    } else {
        if (toe->as.expr->kind != EL_HIR_EXPR_SYMBOL) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-alias",
                alias->target.span, "invalid alias target, symbol expected"
            );
            return NULL;
        }

        if (!el_hir_scope_insert_ex(binder->current_scope, alias->name, toe->as.expr->as.symbol)) {
            return REPORT_REDEFINITION(binder, in->span, alias->name);
        }
    }
    return el_hir_decl_none(binder->arena, in->span);
}

static ElHirDecl* bind_typedef(ElBinder* binder, ElAstDecl* in, ElAstTypedef* typedef_) {
    ElHirSymbol* existing = el_hir_scope_lookup(binder->current_scope, typedef_->name);

    if (existing != NULL) {
        ElHirType* etype = existing->as.type.type;
        if (0
         || existing->kind != EL_SYM_TYPE
         || etype->kind != EL_HIR_TYPE_DISTINCT
         || etype->as.distinct.orig != NULL
        ) {
            return REPORT_REDEFINITION(binder, in->span, typedef_->name);
        }

        if (typedef_->target == NULL) {
            return el_hir_decl_none(binder->arena, in->span);
        }

        ElHirType* target = el_bind_type(binder, typedef_->target);
        if (target == NULL) return NULL;

        ElHirType* incomplete = el_hir_type_unwrap(existing->as.type.type);
        incomplete->as.distinct.orig = target;
        return el_hir_decl_none(binder->arena, in->span);
    }

    ElHirType* distinct = el_hir_new_distinct_type(binder->arena, NULL, typedef_->name);
    ElHirSymbol* the_symbol = el_hir_new_type_symbol(binder->arena, binder->sym_id_counter++, typedef_->name, distinct);

    if (!el_hir_scope_insert(binder->current_scope, the_symbol))
        return REPORT_REDEFINITION(binder, in->span, typedef_->name);

    if (typedef_->target != NULL) {
        ElHirType* target = el_bind_type(binder, typedef_->target);
        if (target == NULL) return NULL;

        distinct->as.distinct.orig = target;
    }

    return el_hir_decl_none(binder->arena, in->span);
}

static ElHirDecl* _bind_decl_internal(ElBinder* binder, ElAstDecl* in) {
    switch (in->type) {
    case EL_AST_DECL_VAR_DEF:
        return bind_var_def(binder, in, &in->as.var_def);
    case EL_AST_DECL_VAR_DECL:
        return bind_var_decl(binder, in, &in->as.var_decl);
    case EL_AST_DECL_FUNC_DEF:
        return bind_func_def(binder, in, &in->as.func_def);
    case EL_AST_DECL_FUNC_DECL:
        return bind_func_decl(binder, in, &in->as.func_decl);
    case EL_AST_DECL_TYPEDEF:
        return bind_typedef(binder, in, &in->as.typedef_);
    case EL_AST_DECL_ALIAS:
        return bind_alias(binder, in, &in->as.alias);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDeclType, in->type);
}

ElHirDecl* el_bind_decl(ElBinder* binder, ElAstDecl* in) {
    if (in == NULL) return NULL;
    el_prof_begin_sub(binder->prof, binder->pss_decl);
    ElHirDecl* result = _bind_decl_internal(binder, in);
    el_prof_finish_sub(binder->prof, binder->pss_decl);
    return result;
}

#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

// TODO: this function is too large and probably should be splitted into smaller helpers
ElHirTopLevelNode* el_binder_bind_toplvl(ElBinder* binder, ElAstTopLevelNode* in) {
    switch (in->type) {
    case EL_AST_TOPLVL_FUNC_DEF: {
        ElAstFuncDef* def = &in->as.func_def;
        _el_binder_push_scope(binder);

        ElType* ret_type = _el_binder_bind_type(binder, def->sig.ret_type);
        if (ret_type == NULL) {
            _el_binder_pop_scope(binder);
            return NULL;
        }

        ElSymbol** params = EL_DYNARENA_NEW_ARR(binder->arena, ElSymbol*, def->sig.params.count);
        usize i = 0;
        for (ElAstFuncParam* param = def->sig.params.head; param != NULL; param = param->next) {
            ElType* type = _el_binder_bind_type(binder, param->type);
            if (type == NULL) {
                _el_binder_pop_scope(binder);
                return NULL;
            }
            
            ElSymbol* psym = el_sema_new_var_symbol(
                binder->arena, binder->sym_id_counter++,
                param->name->name, type
            );
            if (!el_sema_scope_insert(binder->current_scope, psym)) {
                el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.redeclaration",
                    param->span,
                    "redeclaration of parameter '${name}'",
                    EL_DIAG_STRING("name", param->name->name)
                );
            }
            params[i++] = psym;
        }

        ElSymbol* sym = el_sema_new_func_symbol(
            binder->arena, binder->sym_id_counter++, def->sig.name->name,
            ret_type, params, def->sig.params.count
        );
        if (!el_sema_scope_insert(binder->current_scope->parent, sym)) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.redeclaration",
                def->sig.name->span,
                "redeclaration of symbol '${name}'",
                EL_DIAG_STRING("name", def->sig.name->name)
            );
            _el_binder_pop_scope(binder);
            return NULL;
        }

        ElHirBlockStmtNode block = _el_binder_bind_block(binder, def->block);
        ElHirTopLevelNode* func = el_hir_new_func_def(binder->arena, sym, block);
        _el_binder_pop_scope(binder);
        return func;
    }
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTopLevelType, in->type);
}


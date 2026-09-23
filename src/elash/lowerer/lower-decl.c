#include "lowerer-internals.h"

#include <elash/mir/instr.h>
#include <elash/mir/func.h>
#include <elash/mir/module.h>
#include <elash/mir/value.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

static void lower_func_def(ElLowerer* lw, ElHirFuncDef* hir_func) {
    ElMirSymbol* mir_func_sym = el_lowerer_map_symbol(lw, hir_func->symbol);
    ElMirFunc* func = el_mir_new_func(lw->arena, mir_func_sym);

    ElMirFunc* prev_func = lw->current_func;
    uint32_t prev_block_id = lw->current_block_id;

    lw->current_func = func;
    lw->current_block_id = func->block_count++;

    el_mir_ibuf_clear(&lw->ibuf);

    ElMirFuncSymbol* func_sym = &func->symbol->as.func;
    for (uint32_t i = 0; i < func_sym->param_count; i++) {
        ElMirSymbol* param_sym = func_sym->params[i];
        ElMirType* param_type = param_sym->as.var.type;
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, param_type);
        ElMirValue* ptr_reg = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

        el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, ptr_reg, param_type));
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr_reg, func->args[i]));

        lw->symbol_map[param_sym->id] = ptr_reg;
    }

    for (ElHirStmt* node = hir_func->block.stmts; node != NULL; node = node->next) {
        el_lower_stmt(lw, node);
    }

    if (!el_lowerer_has_terminator(lw)) {
        ElMirType* ret_type = func_sym->type->as.func.ret_type;
        if (ret_type->kind == EL_MIR_TYPE_VOID) {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_ret_instr(lw->arena, NULL));
        } else {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_unreachable_instr(lw->arena));
        }
    }

    el_lowerer_emit_block(lw, lw->current_block_id);
    el_mir_module_add_func(lw->current_mod, func);

    lw->current_func = prev_func;
    lw->current_block_id = prev_block_id;
}

static void lower_func_decl(ElLowerer* lw, ElHirFuncDecl* hir_func) {
    if (hir_func->symbol->as.func.is_defined) {
        return;
    }

    ElMirSymbol* mir_func_sym = el_lowerer_map_symbol(lw, hir_func->symbol);
    ElMirType* mir_func_type = mir_func_sym->as.func.type;

    lw->symbol_map[hir_func->symbol->id] = el_mir_new_global(
        lw->arena, mir_func_type, mir_func_sym, NULL, false
    );

    ElMirFunc* func = el_mir_new_func(lw->arena, mir_func_sym);
    el_mir_module_add_func(lw->current_mod, func);
}

static void _lower_global_decl_internal(ElLowerer* lw, ElHirDecl* decl) {
    switch (decl->kind) {
    case EL_HIR_DECL_VAR_DEF: {
        ElHirSymbol* sym = decl->as.var_def.var;
        ElMirType* mir_type = el_tcache_get_mir(lw->tcache, sym->as.var.type);
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
        ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);

        ElMirConstant* init = NULL;
        if (decl->as.var_def.init != NULL) {
            init = _el_lower_const(lw, decl->as.var_def.init);
        }

        lw->symbol_map[sym->id] = el_mir_new_global(lw->arena, ptr_type, mir_sym, init, true);
        break;
    }
    case EL_HIR_DECL_VAR_DECL: {
        ElHirSymbol* sym = decl->as.var_decl.var;
        ElMirType* mir_type = el_tcache_get_mir(lw->tcache, sym->as.var.type);
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
        ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);
        lw->symbol_map[sym->id] = el_mir_new_global(lw->arena, ptr_type, mir_sym, NULL, false);
        break;
    }
    case EL_HIR_DECL_FUNC_DEF: {
        ElMirSymbol* mir_func_sym = el_lowerer_map_symbol(lw, decl->as.func_def.symbol);
        lw->symbol_map[decl->as.func_def.symbol->id] = el_mir_new_global(
            lw->arena, mir_func_sym->as.func.type, mir_func_sym, NULL, true
        );
        lower_func_def(lw, &decl->as.func_def);
        break;
    }
    case EL_HIR_DECL_FUNC_DECL:
        lower_func_decl(lw, &decl->as.func_decl);
        break;
    case EL_HIR_DECL_NONE:
        break;
    }
}

void el_lower_global_decl(ElLowerer* lw, ElHirDecl* decl) {
    el_prof_begin_sub(lw->prof, lw->pss_decl);
    _lower_global_decl_internal(lw, decl);
    el_prof_finish_sub(lw->prof, lw->pss_decl);
}

static void _lower_local_decl_internal(ElLowerer* lw, ElHirDecl* decl) {
    switch (decl->kind) {
    case EL_HIR_DECL_VAR_DEF: {
        // ugly but (i guess) works
        if (decl->as.var_def.scls == EL_STORAGECLS_STATIC) {
            return el_lower_global_decl(lw, decl);
        }

        ElHirSymbol* sym = decl->as.var_def.var;
        ElMirType* type = el_tcache_get_mir(lw->tcache, sym->as.var.type);
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, type);
        ElMirValue* ptr_reg = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

        el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, ptr_reg, type));
        lw->symbol_map[sym->id] = ptr_reg;

        if (decl->as.var_def.init != NULL) {
            if (decl->as.var_def.init->kind == EL_HIR_EXPR_AGGINIT) {
                _el_lower_agginit(lw, ptr_reg, &decl->as.var_def.init->as.agginit);
            } else {
                ElMirValue* init_val = el_lower_expr(lw, decl->as.var_def.init);
                el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr_reg, init_val));
            }
        }
        break;
    }
    case EL_HIR_DECL_VAR_DECL: {
        ElHirSymbol* sym = decl->as.var_decl.var;
        ElMirType* mir_type = el_tcache_get_mir(lw->tcache, sym->as.var.type);
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
        ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);
        lw->symbol_map[sym->id] = el_mir_new_global(lw->arena, ptr_type, mir_sym, NULL, false);
        break;
    }
    case EL_HIR_DECL_FUNC_DEF:
        EL_TODO("local function definitions not supported yet");
        break;
    case EL_HIR_DECL_FUNC_DECL:
        lower_func_decl(lw, &decl->as.func_decl);
        break;
    case EL_HIR_DECL_NONE:
        break;
    }
}

void el_lower_local_decl(ElLowerer* lw, ElHirDecl* decl) {
    el_prof_begin_sub(lw->prof, lw->pss_decl);
    _lower_local_decl_internal(lw, decl);
    el_prof_finish_sub(lw->prof, lw->pss_decl);
}

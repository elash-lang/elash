#include <elash/lowerer/lowerer.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/func.h>

#include <elash/sema/type/prim.h>
#include <elash/sema/symbol/func.h>
#include <elash/sema/symbol/var.h>

// TODO: this function is too large; split it into smaller helpers
//       before adding any new top-level statement types
void el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir) {
    switch (hir->kind) {
    case EL_HIR_TOPLVL_FUNC_DEF: {
        ElHirFuncDef* hir_func = &hir->as.func_def;
        ElHirBlockStmtNode* hir_block = &hir_func->block;

        ElMirFunc* func = el_mir_new_func(lw->arena, hir->as.func_def.symbol);
        lw->current_func = func;
        lw->current_block_id = func->block_count++;

        el_mir_ibuf_clear(&lw->ibuf);

        ElFuncSymbol* func_sym = &func->symbol->as.func;
        for (uint32_t i = 0; i < func_sym->param_count; i++) {
            ElSymbol* param_sym = func_sym->params[i];
            ElType* param_type = param_sym->as.var.type;
            ElType* ptr_type = el_sema_new_ptr_type(lw->arena, param_type);
            ElMirValue* ptr_reg = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

            el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, ptr_reg, param_type));
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr_reg, func->args[i]));

            lw->symbol_map[param_sym->id] = ptr_reg;
        }

        for (ElHirStmtNode* node = hir_block->stmts; node != NULL; node = node->next) {
            el_lowerer_lower_stmt(lw, node);
        }

        if (!el_lowerer_has_terminator(lw)) {
            ElType* ret_type = hir->as.func_def.symbol->as.func.ret_type;
            if (ret_type->kind == EL_TYPE_PRIM && ret_type->as.prim.kind == EL_PRIMTYPE_VOID) {
                el_mir_ibuf_push(&lw->ibuf, el_mir_new_ret_instr(lw->arena, NULL));
            } else {
                el_mir_ibuf_push(&lw->ibuf, el_mir_new_unreachable_instr(lw->arena));
            }
        }
        el_lowerer_emit_block(lw, lw->current_block_id);
        el_mir_module_add_func(lw->current_mod, func);
        return;
    }
    case EL_HIR_TOPLVL_FUNC_DECL: {
        if (hir->as.func_decl.symbol->as.func.is_defined) {
            return;
        }
        ElMirFunc* func = el_mir_new_func(lw->arena, hir->as.func_decl.symbol);
        el_mir_module_add_func(lw->current_mod, func);
        return;
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirTopLevelKind, hir->kind);
}


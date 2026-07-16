#include <elash/lowerer/lowerer.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/module.h>
#include <elash/mir/func.h>
#include <elash/mir/block.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>
#include <elash/mir/instr-buf.h>
#include <elash/mir/value/const.h>
#include <elash/mir/value/reg.h>
#include <elash/mir/instr/binary.h>
#include <elash/mir/instr/return.h>
#include <elash/mir/instr/gep.h>

#include <elash/sema/bin-op.h>

#include <elash/hir/tree/stmt/assign.h>
#include <elash/mir/value/global.h>

#include <stddef.h>

void el_lowerer_init(ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag, ElLowererBuiltins* builtins) {
    lw->arena = arena;
    lw->diag = diag;
    lw->builtins = builtins;
    lw->symbol_map = NULL;
    lw->mir_symbol_map = NULL;
    lw->break_target_id = 0;
    lw->continue_target_id = 0;

    el_mir_ibuf_init(&lw->ibuf);
}

void el_lowerer_free(ElLowerer* lw) {
    el_mir_ibuf_destroy(&lw->ibuf);
}

bool el_lowerer_has_terminator(ElLowerer* lw) {
    if (lw->ibuf.len == 0) return false;
    return el_mir_instr_is_terminator(lw->ibuf.items[lw->ibuf.len - 1]);
}

void el_lowerer_emit_block(ElLowerer* lw, uint32_t id) {
    if (lw->ibuf.len == 0) return;
    ElMirBlock* block = el_mir_new_block_from_ibuf(lw->arena, id, &lw->ibuf);
    el_mir_func_append_block(lw->current_func, block);
    el_mir_ibuf_clear(&lw->ibuf);
}

ElMirValue* el_lowerer_get_lvalue(ElLowerer* lw, ElHirExpr* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_SYMBOL: {
        ElHirSymbol* sym = hir->as.symbol;
        if (sym->kind == EL_SYM_VAR) {
            if (lw->symbol_map && lw->symbol_map[sym->id]) {
                return lw->symbol_map[sym->id];
            }

            ElMirType* mir_type = el_lowerer_map_type(lw, sym->as.var.type);
            ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
            ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);
            ElMirValue* glob = el_mir_new_global(lw->arena, ptr_type, mir_sym);
            if (lw->symbol_map) {
                lw->symbol_map[sym->id] = glob;
            }
            return glob;
        }
        if (sym->kind == EL_SYM_FUNC) {
            ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
            ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);
            return el_mir_new_global(lw->arena, mir_type, mir_sym);
        }
        EL_UNREACHABLE("symbol is not an lvalue (this should be caught during semantic analysis)");
    }

    case EL_HIR_EXPR_BINARY:
        if (hir->as.binary.op == EL_SEMA_BIN_OP_INDEX) {
             ElMirValue* ptr;
             if (hir->as.binary.left->type->kind == EL_HIR_TYPE_RWSLICE) {
                 ptr = el_lowerer_lower_expr(lw, hir->as.binary.left);
             } else if (hir->as.binary.left->type->kind == EL_HIR_TYPE_SLICE) {
                 // slice is { data, len }; index through the data pointer
                 ElMirValue* slice = el_lowerer_lower_expr(lw, hir->as.binary.left);
                 ptr = _el_lowerer_extract_tuple_field(lw, slice, 0);
             } else {
                 ptr = el_lowerer_get_lvalue(lw, hir->as.binary.left);
             }
             ElMirValue* index = el_lowerer_lower_expr(lw, hir->as.binary.right);

             ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
             ElMirType* result_ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
             ElMirValue* res_reg = el_mir_new_reg(lw->arena, result_ptr_type, lw->current_func->reg_count++);
             el_mir_ibuf_push(&lw->ibuf, el_mir_new_gep_instr(lw->arena, res_reg, ptr, index));
             return res_reg;
        }
        break;

    case EL_HIR_EXPR_UNARY:
        if (hir->as.unary.op == EL_SEMA_UNARY_OP_DEREF) {
            // from what i understand, lvalue of *p is effectively the value p
            return el_lowerer_lower_expr(lw, hir->as.unary.operand);
        }
        break;

    default: break;
    }
    EL_UNREACHABLE("expression is not an lvalue (this should be caught during semantic analysis)");
}

ElMirSymbol* el_lowerer_map_symbol(ElLowerer* lw, ElHirSymbol* sym) {
    if (sym == NULL) return NULL;
    if (lw->mir_symbol_map && lw->mir_symbol_map[sym->id]) {
        return lw->mir_symbol_map[sym->id];
    }

    ElMirSymbol* mir_sym = NULL;
    if (sym->kind == EL_SYM_VAR) {
        ElMirType* mir_type = el_lowerer_map_type(lw, sym->as.var.type);
        mir_sym = el_mir_new_var_symbol(lw->arena, sym->id, sym->name, mir_type);
    } else if (sym->kind == EL_SYM_FUNC) {
        ElMirType* ret_type = el_lowerer_map_type(lw, sym->as.func.type->as.func.ret_type);
        ElMirSymbol** params = EL_DYNARENA_NEW_ARR(lw->arena, ElMirSymbol*, sym->as.func.param_count);
        for (usize i = 0; i < sym->as.func.param_count; i++) {
            params[i] = el_lowerer_map_symbol(lw, sym->as.func.params[i]);
        }
        mir_sym = el_mir_new_func_symbol(lw->arena, sym->id, sym->name, ret_type, params, sym->as.func.param_count);
        mir_sym->as.func.is_defined = sym->as.func.is_defined;
    } else {
        EL_UNREACHABLE("only var and func symbols are mapped to MIR");
    }

    if (lw->mir_symbol_map) {
        lw->mir_symbol_map[sym->id] = mir_sym;
    }
    return mir_sym;
}

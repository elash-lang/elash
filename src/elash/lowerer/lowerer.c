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

#include <elash/sema/type/prim.h>
#include <elash/sema/symbol/func.h>
#include <elash/sema/symbol/var.h>
#include <elash/sema/expr/bin-op.h>

#include <elash/mir/value/global.h>

#include <stddef.h>

void el_lowerer_init(ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag) {
    lw->arena = arena;
    lw->diag = diag;
    el_mir_ibuf_init(&lw->ibuf);
    lw->symbol_map = NULL;
}

void el_lowerer_free(ElLowerer* lw) {
    el_mir_ibuf_destroy(&lw->ibuf);
}

ElMirValue* el_lowerer_lower_symbol(ElLowerer* lw, ElSymbol* sym) {
    switch (sym->kind) {
    case EL_SYM_VAR: {
        if (lw->symbol_map && lw->symbol_map[sym->id]) {
            ElMirValue* ptr = lw->symbol_map[sym->id];
            ElMirValue* reg = el_mir_new_reg(lw->arena, sym->as.var.type, lw->current_func->reg_count++);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
            return reg;
        }

        if (lw->current_func != NULL) {
            ElFuncSymbol* func_sym = &lw->current_func->symbol->as.func;
            for (uint32_t i = 0; i < func_sym->param_count; i++) {
                if (func_sym->params[i] == sym) {
                    return lw->current_func->args[i];
                }
            }
        }

        EL_TODO("Variables not supported yet");
    }
    case EL_SYM_FUNC:
        return el_mir_new_global(lw->arena, sym->as.func.type, sym);
    case EL_SYM_TYPE:
        EL_UNREACHABLE("Type symbol in expression context (this should be caught during semantic analysis)");
        break;
    }
    EL_UNREACHABLE_ENUM_VAL(ElSymbolKind, sym->kind);
}

ElMirValue* el_lowerer_lower_expr(ElLowerer* lw, ElHirExprNode* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExprNode* expr = &hir->as.binary;
        ElMirValue* lhs = el_lowerer_lower_expr(lw, expr->left);
        ElMirValue* rhs = el_lowerer_lower_expr(lw, expr->right);

        ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
        ElMirInstr* instr = el_mir_new_bin_instr(lw->arena, reg, expr->op, lhs, rhs);

        el_mir_ibuf_push(&lw->ibuf, instr);
        return reg;
    }
    case EL_HIR_EXPR_UNARY: {
        ElHirUnaryExprNode* expr = &hir->as.unary;
        ElMirValue* operand = el_lowerer_lower_expr(lw, expr->operand);

        ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
        ElMirInstr* instr = el_mir_new_unary_instr(lw->arena, reg, expr->op, operand);

        el_mir_ibuf_push(&lw->ibuf, instr);
        return reg;
    }
    case EL_HIR_EXPR_LITERAL: {
        ElHirLiteral* lit = &hir->as.literal;
        return el_mir_new_const(lw->arena, hir->type, *lit);
    }
    case EL_HIR_EXPR_CALL: {
        ElHirCallExprNode* call = &hir->as.call;

        ElMirValue* callee = el_lowerer_lower_expr(lw, call->callee);
        ElMirValue** args = EL_DYNARENA_NEW_ARR(lw->arena, ElMirValue*, call->arg_count);
        for (usize i = 0; i < call->arg_count; ++i) {
            args[i] = el_lowerer_lower_expr(lw, call->args[i]);
        }

        // TODO: we don't have reference to type_void here...
        bool is_void = (hir->type->kind == EL_TYPE_PRIM && hir->type->as.prim.kind == EL_PRIMTYPE_VOID);
        ElMirValue* result = is_void ? NULL : el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
        ElMirInstr* instr = el_mir_new_call_instr(lw->arena, result, callee, args, call->arg_count);
        el_mir_ibuf_push(&lw->ibuf, instr);
        return result;
    }
    case EL_HIR_EXPR_SYMBOL:
        return el_lowerer_lower_symbol(lw, hir->as.symbol);
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirExprKind, hir->kind);
}

static bool el_lowerer_has_terminator(ElLowerer* lw) {
    if (lw->ibuf.len == 0) return false;
    return el_mir_instr_is_terminator(lw->ibuf.items[lw->ibuf.len - 1]);
}

static void el_lowerer_emit_block(ElLowerer* lw, uint32_t id) {
    if (lw->ibuf.len == 0) return;
    ElMirBlock* block = el_mir_new_block_from_ibuf(lw->arena, id, &lw->ibuf);
    el_mir_func_append_block(lw->current_func, block);
    el_mir_ibuf_clear(&lw->ibuf);
}

void el_lowerer_lower_stmt(ElLowerer* lw, ElHirStmtNode* hir) {
    switch (hir->kind) {
    case EL_HIR_STMT_EXPR:
        el_lowerer_lower_expr(lw, hir->as.expr);
        break;
    case EL_HIR_STMT_RETURN: {
        ElMirValue* ret_val = NULL;
        if (hir->as.return_.value) {
            ret_val = el_lowerer_lower_expr(lw, hir->as.return_.value);
        }
        ElMirInstr* ret_instr = el_mir_new_ret_instr(lw->arena, ret_val);
        el_mir_ibuf_push(&lw->ibuf, ret_instr);
        break;
    }
    case EL_HIR_STMT_VAR_DEF: {
        ElHirVarDefStmtNode* var_def = &hir->as.var_def;
        ElSymbol* sym = var_def->var;

        ElType* ptr_type = el_sema_new_ptr_type(lw->arena, sym->as.var.type);
        ElMirValue* ptr_reg = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

        ElMirInstr* alloca_instr = el_mir_new_alloca_instr(lw->arena, ptr_reg, sym->as.var.type);
        el_mir_ibuf_push(&lw->ibuf, alloca_instr);

        lw->symbol_map[sym->id] = ptr_reg;

        if (var_def->init) {
            ElMirValue* init_val = el_lowerer_lower_expr(lw, var_def->init);
            ElMirInstr* store_instr = el_mir_new_store_instr(lw->arena, ptr_reg, init_val);
            el_mir_ibuf_push(&lw->ibuf, store_instr);
        }
        break;
    }
    case EL_HIR_STMT_BLOCK:
        for (ElHirStmtNode* node = hir->as.block.stmts; node != NULL; node = node->next) {
            el_lowerer_lower_stmt(lw, node);
        }
        break;
    case EL_HIR_STMT_IF: {
        ElHirIfStmtNode* if_stmt = &hir->as.if_;
        ElMirValue* cond = el_lowerer_lower_expr(lw, if_stmt->cond);

        uint32_t then_id = lw->current_func->block_count++;
        uint32_t merge_id = lw->current_func->block_count++;
        uint32_t else_id = (if_stmt->else_ != NULL)
            ? lw->current_func->block_count++
            : merge_id;

        el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, cond, then_id, else_id));
        el_lowerer_emit_block(lw, lw->current_block_id);

        lw->current_block_id = then_id;
        el_lowerer_lower_stmt(lw, if_stmt->then);
        if (!el_lowerer_has_terminator(lw)) {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
        }
        el_lowerer_emit_block(lw, lw->current_block_id);

        if (if_stmt->else_ != NULL) {
            lw->current_block_id = else_id;
            el_lowerer_lower_stmt(lw, if_stmt->else_);
            if (!el_lowerer_has_terminator(lw)) {
                el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
            }
            el_lowerer_emit_block(lw, lw->current_block_id);
        }

        lw->current_block_id = merge_id;
        break;
    }
    }
}

void el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir) {
    switch (hir->kind) {
    case EL_HIR_TOPLVL_FUNC_DEF: {
        ElHirFuncDef* hir_func = &hir->as.func_def;
        ElHirBlockStmtNode* hir_block = &hir_func->block;

        ElMirFunc* func = el_mir_new_func(lw->arena, hir->as.func_def.symbol);
        lw->current_func = func;
        lw->current_block_id = func->block_count++;

        el_mir_ibuf_clear(&lw->ibuf);
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

ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir) {
    lw->current_mod = el_mir_new_module(lw->arena);
    lw->symbol_map = EL_DYNARENA_NEW_ARR_ZEROED(lw->arena, ElMirValue*, hir->sym_count);

    for (ElHirTopLevelNode* node = hir->head; node != NULL; node = node->next) {
        el_lowerer_lower_toplvl(lw, node);
    }

    return lw->current_mod;
}

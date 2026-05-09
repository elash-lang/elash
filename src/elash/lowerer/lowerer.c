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
#include <elash/sema/expr/bin-op.h>

#include <stddef.h>

void el_lowerer_init(ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag) {
    lw->arena = arena;
    lw->diag = diag;
    el_mir_ibuf_init(&lw->ibuf);
}

void el_lowerer_free(ElLowerer* lw) {
    el_mir_ibuf_destroy(&lw->ibuf);
}

ElMirValue* el_lowerer_lower_symbol(ElLowerer* lw, ElSymbol* sym) {
    EL_TODO("Implement symbol lowering");
}

// TODO: stub 
ElMirValue* el_lowerer_lower_expr(ElLowerer* lw, ElHirExprNode* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExprNode* expr = &hir->as.binary;
        ElMirValue* lhs = el_lowerer_lower_expr(lw, expr->left);
        ElMirValue* rhs = el_lowerer_lower_expr(lw, expr->right);

        ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->next_reg_id++);
        ElMirInstr* instr = el_mir_new_bin_instr(lw->arena, reg, expr->op, lhs, rhs);

        el_mir_ibuf_push(&lw->ibuf, instr);
        return reg;
    }
    case EL_HIR_EXPR_UNARY: {
        ElHirUnaryExprNode* expr = &hir->as.unary;
        ElMirValue* operand = el_lowerer_lower_expr(lw, expr->operand);

        ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->next_reg_id++);
        ElMirInstr* instr = el_mir_new_unary_instr(lw->arena, reg, expr->op, operand);

        el_mir_ibuf_push(&lw->ibuf, instr);
        return reg;
    }
    case EL_HIR_EXPR_LITERAL: {
        ElHirLiteral* lit = &hir->as.literal;
        return el_mir_new_const(lw->arena, hir->type, *lit);
    }
    case EL_HIR_EXPR_CALL: {
        EL_TODO("Implement function calls lowering");
    }
    case EL_HIR_EXPR_SYMBOL:
        return el_lowerer_lower_symbol(lw, hir->as.symbol);
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirExprKind, hir->kind);
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
    case EL_HIR_STMT_BLOCK:
        for (ElHirStmtNode* node = hir->as.block.stmts; node != NULL; node = node->next) {
            el_lowerer_lower_stmt(lw, node);
        }
        break;
    }
}

void el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir) {
    switch (hir->kind) {
    case EL_HIR_TOPLVL_FUNC_DEF: {
        ElHirFuncDefinition* hir_func = &hir->as.func_def;
        ElHirBlockStmtNode* hir_block = &hir_func->block;
        
        ElMirFunc* func = el_mir_new_func(lw->arena, hir->as.func_def.symbol);
        lw->current_func = func;
        el_mir_ibuf_clear(&lw->ibuf);
        for (ElHirStmtNode* node = hir_block->stmts; node != NULL; node = node->next) {
            el_lowerer_lower_stmt(lw, node);
        }

        ElMirBlock* block = el_mir_new_block_from_ibuf(lw->arena, func->next_block_id++, &lw->ibuf);
        el_mir_func_append_block(func, block);

        el_mir_module_add_func(lw->current_mod, func);
    }
    }
}

ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir) { 
    lw->current_mod = el_mir_new_module(lw->arena);
    for (ElHirTopLevelNode* node = hir->head; node != NULL; node = node->next) {
        el_lowerer_lower_toplvl(lw, node);
    }
   
    return lw->current_mod;
}

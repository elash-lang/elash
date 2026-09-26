#include "lowerer-internals.h"

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/block.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>
#include <elash/mir/value/const.h>
#include <elash/mir/value/reg.h>
#include <elash/mir/type.h>

#include <elash/hir/tree/stmt/break.h>
#include <elash/hir/tree/stmt/continue.h>

static void lower_if(ElLowerer* lw, ElHirIfStmt* if_stmt) {
    ElMirValue* cond = el_lower_expr(lw, if_stmt->cond);

    uint32_t then_id = new_block_id(lw);
    uint32_t merge_id = new_block_id(lw);
    uint32_t else_id = (if_stmt->else_ != NULL)
        ? new_block_id(lw)
        : merge_id;

    emit_jmpif(lw, cond, then_id, else_id);
    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = then_id;
    el_lower_stmt(lw, if_stmt->then);
    if (!el_lowerer_has_terminator(lw)) {
        emit_jmp(lw, merge_id);
    }
    el_lowerer_emit_block(lw, lw->current_block_id);

    if (if_stmt->else_ != NULL) {
        lw->current_block_id = else_id;
        el_lower_stmt(lw, if_stmt->else_);
        if (!el_lowerer_has_terminator(lw)) {
            emit_jmp(lw, merge_id);
        }
        el_lowerer_emit_block(lw, lw->current_block_id);
    }

    lw->current_block_id = merge_id;
}

static void lower_break(ElLowerer* lw, ElHirBreakStmt* node) {
    (void) node;
    emit_jmp(lw, lw->break_target_id);
}

static void lower_continue(ElLowerer* lw, ElHirContinueStmt* node) {
    (void) node;
    emit_jmp(lw, lw->continue_target_id);
}

static void lower_while(ElLowerer* lw, ElHirWhileStmt* while_stmt) {
    uint32_t cond_id = new_block_id(lw);
    uint32_t body_id = new_block_id(lw);
    uint32_t exit_id = new_block_id(lw);

    uint32_t prev_break = lw->break_target_id;
    uint32_t prev_continue = lw->continue_target_id;
    lw->break_target_id = exit_id;
    lw->continue_target_id = cond_id;

    emit_jmp(lw, cond_id);
    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = cond_id;
    ElMirValue* cond = el_lower_expr(lw, while_stmt->cond);

    emit_jmpif(lw, cond, body_id, exit_id);

    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = body_id;
    el_lower_stmt(lw, while_stmt->body);

    if (!el_lowerer_has_terminator(lw)) {
        emit_jmp(lw, cond_id);
    }

    el_lowerer_emit_block(lw, lw->current_block_id);
    lw->current_block_id = exit_id;

    lw->break_target_id = prev_break;
    lw->continue_target_id = prev_continue;
}

static void lower_assign(ElLowerer* lw, ElHirAssignStmt* assign) {
    ElMirValue* value = el_lower_expr(lw, assign->value);
    ElMirValue* ptr = el_lowerer_get_lvalue(lw, assign->target);
    emit_store(lw, ptr, value);
}

static void lower_cassign(ElLowerer* lw, ElHirCompoundAssignStmt* cassign) {
    ElMirValue* ptr = el_lowerer_get_lvalue(lw, cassign->target);
    ElMirType* target_mir_type = el_tcache_get_mir(lw->tcache, cassign->target->type);

    if (cassign->op == EL_BIN_OP_AND || cassign->op == EL_BIN_OP_OR || cassign->op == EL_BIN_OP_IMP) {
        ElMirValue* current_val = emit_load(lw, target_mir_type, ptr);

        uint32_t rhs_id = new_block_id(lw);
        uint32_t merge_id = new_block_id(lw);

        if (cassign->op == EL_BIN_OP_AND) {
            emit_jmpif(lw, current_val, rhs_id, merge_id);
            el_lowerer_emit_block(lw, lw->current_block_id);
        } else if (cassign->op == EL_BIN_OP_OR) {
            emit_jmpif(lw, current_val, merge_id, rhs_id);
            el_lowerer_emit_block(lw, lw->current_block_id);
        } else if (cassign->op == EL_BIN_OP_IMP) {
            uint32_t lhs_false_id = new_block_id(lw);
            emit_jmpif(lw, current_val, rhs_id, lhs_false_id);
            el_lowerer_emit_block(lw, lw->current_block_id);

            lw->current_block_id = lhs_false_id;
            ElMirConstant true_lit = { .kind = EL_MIR_CONST_INT, .as.int_ = EL_INT128(1) };
            ElMirValue* true_val = el_mir_new_const(lw->arena, target_mir_type, true_lit);
            emit_store(lw, ptr, true_val);
            emit_jmp(lw, merge_id);
            el_lowerer_emit_block(lw, lw->current_block_id);
        }

        lw->current_block_id = rhs_id;
        ElMirValue* rhs = el_lower_expr(lw, cassign->value);
        emit_store(lw, ptr, rhs);
        emit_jmp(lw, merge_id);
        el_lowerer_emit_block(lw, lw->current_block_id);

        lw->current_block_id = merge_id;
        return;
    }

    // Load current value
    ElMirValue* current_val = emit_load(lw, target_mir_type, ptr);

    // Lower RHS
    ElMirValue* rhs = el_lower_expr(lw, cassign->value);

    // Perform op
    ElMirValue* result = emit_reg(lw, target_mir_type);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, result, cassign->op, current_val, rhs));

    // Store back
    emit_store(lw, ptr, result);
}

static void lower_return(ElLowerer* lw, ElHirReturnStmt* ret) {
    ElMirValue* ret_val = ret->value != NULL
        ? el_lower_expr(lw, ret->value)
        : NULL;

    ElMirInstr* ret_instr = el_mir_new_ret_instr(lw->arena, ret_val);
    el_mir_ibuf_push(&lw->ibuf, ret_instr);
}

static void _lower_stmt_internal(ElLowerer* lw, ElHirStmt* hir) {
    if (el_lowerer_has_terminator(lw)) return;

    switch (hir->kind) {
    case EL_HIR_STMT_IF:       return lower_if(lw, &hir->as.if_);
    case EL_HIR_STMT_WHILE:    return lower_while(lw, &hir->as.while_);
    case EL_HIR_STMT_BREAK:    return lower_break(lw, &hir->as.break_);
    case EL_HIR_STMT_CONTINUE: return lower_continue(lw, &hir->as.continue_);
    case EL_HIR_STMT_ASSIGN:   return lower_assign(lw, &hir->as.assign);
    case EL_HIR_STMT_RETURN:   return lower_return(lw, &hir->as.return_);
    case EL_HIR_STMT_CASSIGN:  return lower_cassign(lw, &hir->as.cassign);

    case EL_HIR_STMT_BLOCK:
        for (ElHirStmt* node = hir->as.block.stmts; node != NULL; node = node->next) {
            el_lower_stmt(lw, node);
        }
        return;

    case EL_HIR_STMT_DECL:
        for (ElHirDecl* d = hir->as.decl; d != NULL; d = d->next) {
            el_lower_local_decl(lw, d);
        }
        return;

    case EL_HIR_STMT_EXPR:
        el_lower_expr(lw, hir->as.expr);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirStmtKind, hir->kind);
}

void el_lower_stmt(ElLowerer* lw, ElHirStmt* hir) {
    el_prof_begin_sub(lw->prof, lw->pss_stmt);
    _lower_stmt_internal(lw, hir);
    el_prof_finish_sub(lw->prof, lw->pss_stmt);
}

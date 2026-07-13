#include <elash/lowerer/lowerer.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/block.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>

#include <elash/hir/tree/stmt/break.h>
#include <elash/hir/tree/stmt/continue.h>

void _el_lowerer_lower_if(ElLowerer* lw, ElHirIfStmt* if_stmt) {
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
}

void _el_lowerer_lower_break(ElLowerer* lw, ElHirBreakStmt* node) {
    (void) node;
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, lw->break_target_id));
}

void _el_lowerer_lower_continue(ElLowerer* lw, ElHirContinueStmt* node) {
    (void) node;
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, lw->continue_target_id));
}

void _el_lowerer_lower_while(ElLowerer* lw, ElHirWhileStmt* while_stmt) {
    uint32_t cond_id = lw->current_func->block_count++;
    uint32_t body_id = lw->current_func->block_count++;
    uint32_t exit_id = lw->current_func->block_count++;

    uint32_t prev_break = lw->break_target_id;
    uint32_t prev_continue = lw->continue_target_id;
    lw->break_target_id = exit_id;
    lw->continue_target_id = cond_id;

    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, cond_id));
    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = cond_id;
    ElMirValue* cond = el_lowerer_lower_expr(lw, while_stmt->cond);

    el_mir_ibuf_push(
        &lw->ibuf,
        el_mir_new_jmpif_instr(
            lw->arena,
            cond,
            body_id,
            exit_id
        )
    );

    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = body_id;
    el_lowerer_lower_stmt(lw, while_stmt->body);

    if (!el_lowerer_has_terminator(lw)) {
        el_mir_ibuf_push(
            &lw->ibuf,
            el_mir_new_jmp_instr(lw->arena, cond_id)
        );
    }

    el_lowerer_emit_block(lw, lw->current_block_id);
    lw->current_block_id = exit_id;

    lw->break_target_id = prev_break;
    lw->continue_target_id = prev_continue;
}

void _el_lowerer_lower_assign(ElLowerer* lw, ElHirAssignStmt* assign) {
    ElMirValue* value = el_lowerer_lower_expr(lw, assign->value);
    ElMirValue* ptr = el_lowerer_get_lvalue(lw, assign->target);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, value));
}

void _el_lowerer_lower_cassign(ElLowerer* lw, ElHirCompoundAssignStmt* cassign) {
    ElMirValue* ptr = el_lowerer_get_lvalue(lw, cassign->target);

    if (cassign->op == EL_SEMA_BIN_OP_AND || cassign->op == EL_SEMA_BIN_OP_OR || cassign->op == EL_SEMA_BIN_OP_IMP) {
        ElMirValue* current_val = el_mir_new_reg(lw->arena, cassign->target->type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, current_val, ptr));

        uint32_t rhs_id = lw->current_func->block_count++;
        uint32_t merge_id = lw->current_func->block_count++;

        if (cassign->op == EL_SEMA_BIN_OP_AND) {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, current_val, rhs_id, merge_id));
            el_lowerer_emit_block(lw, lw->current_block_id);

            lw->current_block_id = rhs_id;
            ElMirValue* rhs = el_lowerer_lower_expr(lw, cassign->value);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, rhs));
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
            el_lowerer_emit_block(lw, lw->current_block_id);
        } else if (cassign->op == EL_SEMA_BIN_OP_OR) {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, current_val, merge_id, rhs_id));
            el_lowerer_emit_block(lw, lw->current_block_id);

            lw->current_block_id = rhs_id;
            ElMirValue* rhs = el_lowerer_lower_expr(lw, cassign->value);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, rhs));
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
            el_lowerer_emit_block(lw, lw->current_block_id);
        } else if (cassign->op == EL_SEMA_BIN_OP_IMP) {
            uint32_t lhs_false_id = lw->current_func->block_count++;
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, current_val, rhs_id, lhs_false_id));
            el_lowerer_emit_block(lw, lw->current_block_id);

            lw->current_block_id = lhs_false_id;
            ElMirValue* true_val = el_mir_new_const(lw->arena, cassign->target->type, (ElConstant) { .as.bool_ = true });
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, true_val));
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
            el_lowerer_emit_block(lw, lw->current_block_id);

            lw->current_block_id = rhs_id;
            ElMirValue* rhs = el_lowerer_lower_expr(lw, cassign->value);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, rhs));
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
            el_lowerer_emit_block(lw, lw->current_block_id);
        }

        lw->current_block_id = merge_id;
        return;
    }

    // Load current value
    ElMirValue* current_val = el_mir_new_reg(lw->arena, cassign->target->type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, current_val, ptr));

    // Lower RHS
    ElMirValue* rhs = el_lowerer_lower_expr(lw, cassign->value);

    // Perform op
    ElMirValue* result = el_mir_new_reg(lw->arena, cassign->target->type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, result, cassign->op, current_val, rhs));

    // Store back
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, result));
}

void _el_lowerer_lower_return(ElLowerer* lw, ElHirReturnStmt* ret) {
    ElMirValue* ret_val = ret->value != NULL
        ? el_lowerer_lower_expr(lw, ret->value)
        : NULL;
    ElMirInstr* ret_instr = el_mir_new_ret_instr(lw->arena, ret_val);
    el_mir_ibuf_push(&lw->ibuf, ret_instr);
}

void el_lowerer_lower_stmt(ElLowerer* lw, ElHirStmt* hir) {
    if (el_lowerer_has_terminator(lw)) return;

    switch (hir->kind) {
    case EL_HIR_STMT_EXPR: el_lowerer_lower_expr(lw, hir->as.expr); return;
    case EL_HIR_STMT_BLOCK:
        for (ElHirStmt* node = hir->as.block.stmts; node != NULL; node = node->next) {
            el_lowerer_lower_stmt(lw, node);
        }
        return;

    case EL_HIR_STMT_ASSIGN:  return _el_lowerer_lower_assign(lw, &hir->as.assign);
    case EL_HIR_STMT_RETURN:  return _el_lowerer_lower_return(lw, &hir->as.return_);
    case EL_HIR_STMT_DECL:    return _el_lowerer_lower_local_decl(lw, hir->as.decl);

    case EL_HIR_STMT_COMPOUND_ASSIGN: return _el_lowerer_lower_cassign(lw, &hir->as.cassign);

    case EL_HIR_STMT_IF:      return _el_lowerer_lower_if(lw, &hir->as.if_);
    case EL_HIR_STMT_WHILE:   return _el_lowerer_lower_while(lw, &hir->as.while_);

    case EL_HIR_STMT_BREAK:    return _el_lowerer_lower_break(lw, &hir->as.break_);
    case EL_HIR_STMT_CONTINUE: return _el_lowerer_lower_continue(lw, &hir->as.continue_);
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirStmtKind, hir->kind);
}

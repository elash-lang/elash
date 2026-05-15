#include <elash/lowerer/lowerer.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/block.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>

// TODO: this function is to large and probably should be splitted into smaller helpers
//       "clang-tidy: Function 'el_lowerer_lower_stmt' has cognitive complexity of 31 (threshold 25)" ~2026
void el_lowerer_lower_stmt(ElLowerer* lw, ElHirStmtNode* hir) {
    switch (hir->kind) {
    case EL_HIR_STMT_EXPR:
        el_lowerer_lower_expr(lw, hir->as.expr);
        break;
    case EL_HIR_STMT_ASSIGN: {
        ElHirAssignStmtNode* assign = &hir->as.assign;
        ElMirValue* value = el_lowerer_lower_expr(lw, assign->value);

        if (assign->target->kind == EL_HIR_EXPR_SYMBOL) {
            ElSymbol* sym = assign->target->as.symbol;
            if (lw->symbol_map && lw->symbol_map[sym->id]) {
                ElMirValue* ptr = lw->symbol_map[sym->id];
                el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, value));
            } else {
                EL_UNREACHABLE("invalid assignment target");
            }
        } else {
            EL_TODO("support for pointer dereferences etc.");
        }
        break;
    }
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

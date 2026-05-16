#include <elash/lowerer/lowerer.h>

#include <elash/sema/expr/bin-op.h>
#include <elash/sema/expr/unary-op.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/instr.h>
#include <elash/mir/value/const.h>

ElMirValue* el_lowerer_get_lvalue(ElLowerer* lw, ElHirExprNode* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_SYMBOL: {
        ElSymbol* sym = hir->as.symbol;
        if (sym->kind == EL_SYM_VAR) {
            if (lw->symbol_map && lw->symbol_map[sym->id]) {
                return lw->symbol_map[sym->id];
            }
            EL_TODO("global variables not supported yet");
        }
        EL_UNREACHABLE("symbol is not an lvalue (this should be caught during semantic analysis)");
    }
    
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

ElMirValue* el_lowerer_lower_symbol(ElLowerer* lw, ElSymbol* sym, ElType* type) {
    switch (sym->kind) {
    case EL_SYM_VAR: {
        ElMirValue* ptr = lw->symbol_map[sym->id];
        ElMirValue* reg = el_mir_new_reg(lw->arena, type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
        return reg;
    }
    case EL_SYM_FUNC:
        return el_mir_new_global(lw->arena, type, sym);
    case EL_SYM_TYPE:
        EL_UNREACHABLE("Type symbol in expression context (this should be caught during semantic analysis)");
        break;
    }
    EL_UNREACHABLE_ENUM_VAL(ElSymbolKind, sym->kind);
}

static bool _el_lowerer_is_incdec(ElSemaUnaryOp op) {
    return op >= EL_SEMA_UNARY_OP_PRE_INC && op <= EL_SEMA_UNARY_OP_POST_DEC;
}

static ElMirValue* _el_lowerer_lower_incdec(ElLowerer* lw, ElHirUnaryExprNode* expr) {
    ElMirValue* ptr = el_lowerer_get_lvalue(lw, expr->operand);
    ElType*     val_type = expr->operand->type;

    ElMirValue* current = el_mir_new_reg(lw->arena, val_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, current, ptr));

    ElHirLiteral one_lit = { .as.int_ = 1 };
    ElMirValue* one = el_mir_new_const(lw->arena, val_type, one_lit);

    ElSemaBinOp bin_op = (expr->op == EL_SEMA_UNARY_OP_PRE_INC || expr->op == EL_SEMA_UNARY_OP_POST_INC)
        ? EL_SEMA_BIN_OP_ADD
        : EL_SEMA_BIN_OP_SUB;

    ElMirValue* updated = el_mir_new_reg(lw->arena, val_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, updated, bin_op, current, one));
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, updated));

    return el_sema_unary_op_is_post(expr->op) ? current : updated;
}

// TODO: this function is too large; split it into smaller helpers
//       before adding any new expression types
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

        if (expr->op == EL_SEMA_UNARY_OP_ADDROF) {
            return el_lowerer_get_lvalue(lw, expr->operand);
        }

        if (expr->op == EL_SEMA_UNARY_OP_DEREF) {
            ElMirValue* ptr = el_lowerer_lower_expr(lw, expr->operand);
            ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
            return reg;
        }

        if (_el_lowerer_is_incdec(expr->op)) {
            return _el_lowerer_lower_incdec(lw, expr);
        }

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

        bool is_void = el_sema_type_eql(hir->type, lw->builtins->type_void);
        ElMirValue* result = is_void ? NULL : el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
        ElMirInstr* instr = el_mir_new_call_instr(lw->arena, result, callee, args, call->arg_count);
        el_mir_ibuf_push(&lw->ibuf, instr);
        return result;
    }
    case EL_HIR_EXPR_SYMBOL:
        return el_lowerer_lower_symbol(lw, hir->as.symbol, hir->type);
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirExprKind, hir->kind);
}


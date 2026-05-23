#include <elash/lowerer/lowerer.h>

#include <elash/sema/expr/bin-op.h>
#include <elash/sema/expr/unary-op.h>
#include <elash/sema/type/ptr.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/instr.h>
#include <elash/mir/value/const.h>

ElMirValue* el_lowerer_lower_symbol(ElLowerer* lw, ElSymbol* sym, ElType* type) {
    if (lw->symbol_map && lw->symbol_map[sym->id]) {
        ElMirValue* val = lw->symbol_map[sym->id];
        if (sym->kind == EL_SYM_VAR) {
            ElMirValue* reg = el_mir_new_reg(lw->arena, type, lw->current_func->reg_count++);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, val));
            return reg;
        }
        return val;
    }

    switch (sym->kind) {
    case EL_SYM_VAR: {
        ElType* ptr_type = el_sema_new_ptr_type(lw->arena, type);
        ElMirValue* glob = el_mir_new_global(lw->arena, ptr_type, sym);
        if (lw->symbol_map) lw->symbol_map[sym->id] = glob;

        ElMirValue* reg = el_mir_new_reg(lw->arena, type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, glob));
        return reg;
    }
    case EL_SYM_FUNC: {
        ElMirValue* glob = el_mir_new_global(lw->arena, type, sym);
        if (lw->symbol_map) lw->symbol_map[sym->id] = glob;
        return glob;
    }
    case EL_SYM_BUILTIN:
        EL_UNREACHABLE("builtin functions cannot be used as values");
        return NULL;
    case EL_SYM_TYPE:
        EL_UNREACHABLE("Type symbol in expression context (this should be caught during semantic analysis)");
        break;
    }
    EL_UNREACHABLE_ENUM_VAL(ElSymbolKind, sym->kind);
}

static bool _el_lowerer_is_incdec(ElSemaUnaryOp op) {
    return op >= EL_SEMA_UNARY_OP_PRE_INC && op <= EL_SEMA_UNARY_OP_POST_DEC;
}

static ElMirValue* _el_lowerer_lower_incdec(ElLowerer* lw, ElHirUnaryExpr* expr) {
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

ElMirValue* _el_lowerer_lower_bin_expr(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin) {
    if (bin->op == EL_SEMA_BIN_OP_INDEX) {
         ElMirValue* ptr = el_lowerer_get_lvalue(lw, hir);
         ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
         el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
         return reg;
    }
    ElMirValue* lhs = el_lowerer_lower_expr(lw, bin->left);
    ElMirValue* rhs = el_lowerer_lower_expr(lw, bin->right);

    ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
    ElMirInstr* instr = el_mir_new_bin_instr(lw->arena, reg, bin->op, lhs, rhs);

    el_mir_ibuf_push(&lw->ibuf, instr);
    return reg;
}

ElMirValue* _el_lowerer_lower_unary_expr(ElLowerer* lw, ElHirExpr* hir, ElHirUnaryExpr* unary) {
    if (unary->op == EL_SEMA_UNARY_OP_ADDROF) {
        return el_lowerer_get_lvalue(lw, unary->operand);
    }

    if (unary->op == EL_SEMA_UNARY_OP_DEREF) {
        ElMirValue* ptr = el_lowerer_lower_expr(lw, unary->operand);
        ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
        return reg;
    }

    if (_el_lowerer_is_incdec(unary->op)) {
        return _el_lowerer_lower_incdec(lw, unary);
    }

    ElMirValue* operand = el_lowerer_lower_expr(lw, unary->operand);

    ElMirValue* reg = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
    ElMirInstr* instr = el_mir_new_unary_instr(lw->arena, reg, unary->op, operand);

    el_mir_ibuf_push(&lw->ibuf, instr);
    return reg;
}

ElMirValue* _el_lowerer_lower_call_expr(ElLowerer* lw, ElHirExpr* hir, ElHirCallExpr* call) {
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

ElMirValue* _el_lowerer_lower_array_lit_expr(ElLowerer* lw, ElHirExpr* hir) {
    ElType* ptr_type = el_sema_new_ptr_type(lw->arena, hir->type);
    ElMirValue* ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, ptr, hir->type));

    _el_lowerer_lower_array_lit(lw, ptr, &hir->as.array_lit);

    ElMirValue* res = el_mir_new_reg(lw->arena, hir->type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, res, ptr));
    return res;
}

ElMirValue* el_lowerer_lower_expr(ElLowerer* lw, ElHirExpr* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_BINARY:        return _el_lowerer_lower_bin_expr(lw, hir, &hir->as.binary);
    case EL_HIR_EXPR_UNARY:         return _el_lowerer_lower_unary_expr(lw, hir, &hir->as.unary);
    case EL_HIR_EXPR_CALL:          return _el_lowerer_lower_call_expr(lw, hir, &hir->as.call);
    case EL_HIR_EXPR_ARRAY_LITERAL: return _el_lowerer_lower_array_lit_expr(lw, hir);
    case EL_HIR_EXPR_SYMBOL:        return el_lowerer_lower_symbol(lw, hir->as.symbol, hir->type);
    case EL_HIR_EXPR_LITERAL:       return el_mir_new_const(lw->arena, hir->type, hir->as.literal);
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirExprKind, hir->kind);
}

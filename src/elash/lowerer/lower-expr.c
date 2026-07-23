#include <elash/lowerer/lowerer.h>
#include <elash/lowerer/builtin.h>

#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/instr.h>
#include <elash/mir/type.h>

#include <elash/mir/value/global.h>
#include <elash/mir/value/const.h>
#include <elash/mir/value/reg.h>

static inline bool is_int_or_char(ElMirType* type) {
    return type->kind == EL_MIR_TYPE_INT;
}

static bool el_hir_expr_is_lvalue(const ElHirExpr* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_SYMBOL:
        return hir->as.symbol->kind == EL_SYM_VAR || hir->as.symbol->kind == EL_SYM_FUNC;
    case EL_HIR_EXPR_BINARY:
        return hir->as.binary.op == EL_SEMA_BIN_OP_INDEX;
    case EL_HIR_EXPR_UNARY:
        return hir->as.unary.op == EL_SEMA_UNARY_OP_DEREF;
    case EL_HIR_EXPR_MEMBER:
        return el_hir_expr_is_lvalue(hir->as.member.expr);
    case EL_HIR_EXPR_TMEMBER:
        return el_hir_expr_is_lvalue(hir->as.tmember.expr);
    default:
        return false;
    }
}

ElMirValue* _el_lowerer_lower_cast_expr(ElLowerer* lw, ElHirExpr* hir) {
    ElMirValue* operand = el_lowerer_lower_expr(lw, hir->as.cast.expr);
    ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
    ElMirValue* result = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);

    ElMirType* from_type = el_lowerer_map_type(lw, hir->as.cast.expr->type);
    bool from_is_integral = is_int_or_char(from_type);
    bool to_is_integral = is_int_or_char(mir_type);
    bool from_is_float = from_type->kind == EL_MIR_TYPE_FLOAT;
    bool to_is_float = mir_type->kind == EL_MIR_TYPE_FLOAT;

    if (from_is_integral && to_is_integral) {
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_intcast_instr(lw->arena, result, operand));
    } else if (from_is_float || to_is_float) {
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_fpcast_instr(lw->arena, result, operand));
    } else {
        // TODO: implement sizeof
        /* EL_ASSERT(sizeof hir->type == sizeof hir->as.cast.expr->type or sth) */;
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_bitcast_instr(lw->arena, result, operand));
    }
    return result;
}

ElMirValue* el_lowerer_lower_symbol(ElLowerer* lw, ElHirSymbol* sym, const ElHirType* hir_type) {
    ElMirType* type = el_lowerer_map_type(lw, hir_type);
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
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, type);
        ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);
        ElMirValue* glob = el_mir_new_global(lw->arena, ptr_type, mir_sym);
        if (lw->symbol_map) lw->symbol_map[sym->id] = glob;

        ElMirValue* reg = el_mir_new_reg(lw->arena, type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, glob));
        return reg;
    }
    case EL_SYM_FUNC: {
        ElMirSymbol* mir_sym = el_lowerer_map_symbol(lw, sym);
        ElMirValue* glob = el_mir_new_global(lw->arena, type, mir_sym);
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
    EL_UNREACHABLE_ENUM_VAL(ElHirSymbolKind, sym->kind);
}

static bool _el_lowerer_is_incdec(ElSemaUnaryOp op) {
    return op >= EL_SEMA_UNARY_OP_PRE_INC && op <= EL_SEMA_UNARY_OP_POST_DEC;
}

static ElMirValue* _el_lowerer_lower_incdec(ElLowerer* lw, ElHirUnaryExpr* expr) {
    ElMirValue* ptr = el_lowerer_get_lvalue(lw, expr->operand);
    ElMirType*  val_type = el_lowerer_map_type(lw, expr->operand->type);

    ElMirValue* current = el_mir_new_reg(lw->arena, val_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, current, ptr));

    ElMirConstant one_lit;
    if (val_type->kind == EL_MIR_TYPE_FLOAT) {
        one_lit.as.float_ = 1.0;
    } else {
        one_lit.as.int_ = 1;
    }
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
    ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
    if (bin->op == EL_SEMA_BIN_OP_INDEX) {
         ElMirValue* ptr = el_lowerer_get_lvalue(lw, hir);
         ElMirValue* reg = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
         el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
         return reg;
    }

    if (bin->op == EL_SEMA_BIN_OP_AND || bin->op == EL_SEMA_BIN_OP_OR || bin->op == EL_SEMA_BIN_OP_IMP) {
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
        ElMirValue* res_ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, res_ptr, mir_type));

        ElMirValue* lhs = el_lowerer_lower_expr(lw, bin->left);

        if (bin->op == EL_SEMA_BIN_OP_IMP) {
            ElMirConstant true_lit = { .as.int_ = 1 };
            ElMirValue* true_val = el_mir_new_const(lw->arena, mir_type, true_lit);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, res_ptr, true_val));
        } else {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, res_ptr, lhs));
        }

        uint32_t rhs_id = lw->current_func->block_count++;
        uint32_t merge_id = lw->current_func->block_count++;

        if (bin->op == EL_SEMA_BIN_OP_AND || bin->op == EL_SEMA_BIN_OP_IMP) {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, lhs, rhs_id, merge_id));
        } else {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, lhs, merge_id, rhs_id));
        }
        el_lowerer_emit_block(lw, lw->current_block_id);

        lw->current_block_id = rhs_id;
        ElMirValue* rhs = el_lowerer_lower_expr(lw, bin->right);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, res_ptr, rhs));
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
        el_lowerer_emit_block(lw, lw->current_block_id);

        lw->current_block_id = merge_id;
        ElMirValue* res_val = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, res_val, res_ptr));
        return res_val;
    }

    ElMirValue* lhs = el_lowerer_lower_expr(lw, bin->left);
    ElMirValue* rhs = el_lowerer_lower_expr(lw, bin->right);

    ElMirValue* reg = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
    ElMirInstr* instr = el_mir_new_bin_instr(lw->arena, reg, bin->op, lhs, rhs);

    el_mir_ibuf_push(&lw->ibuf, instr);
    return reg;
}

ElMirValue* _el_lowerer_lower_unary_expr(ElLowerer* lw, ElHirExpr* hir, ElHirUnaryExpr* unary) {
    ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
    if (unary->op == EL_SEMA_UNARY_OP_ADDROF) {
        return el_lowerer_get_lvalue(lw, unary->operand);
    }

    if (unary->op == EL_SEMA_UNARY_OP_DEREF) {
        ElMirValue* ptr = el_lowerer_lower_expr(lw, unary->operand);
        ElMirValue* reg = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
        return reg;
    }

    if (_el_lowerer_is_incdec(unary->op)) {
        return _el_lowerer_lower_incdec(lw, unary);
    }

    ElMirValue* operand = el_lowerer_lower_expr(lw, unary->operand);

    ElMirValue* reg = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
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

    ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
    bool is_void = el_mir_type_eql(mir_type, lw->builtins->type_void);
    ElMirValue* result = is_void ? NULL : el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
    ElMirInstr* instr = el_mir_new_call_instr(lw->arena, result, callee, args, call->arg_count);
    el_mir_ibuf_push(&lw->ibuf, instr);
    return result;
}

ElMirValue* _el_lowerer_lower_array_lit_expr(ElLowerer* lw, ElHirExpr* hir) {
    ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
    ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
    ElMirValue* ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, ptr, mir_type));

    _el_lowerer_lower_array_lit(lw, ptr, &hir->as.array_lit);

    ElMirValue* res = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, res, ptr));
    return res;
}

ElMirValue* _el_lowerer_lower_intr_expr(ElLowerer* lw, ElHirExpr* hir) {
    switch (hir->as.intr.kind) {
    case EL_HIR_INTR_SLICE_LEN: {
        ElMirValue* slice = el_lowerer_lower_expr(lw, hir->as.intr.params.slice);
        return _el_lowerer_extract_tuple_field(lw, slice, EL_MIR_SLICE_FIELD_LEN);
    }
    case EL_HIR_INTR_SLICE_DATA: {
        ElMirValue* slice = el_lowerer_lower_expr(lw, hir->as.intr.params.slice);
        return _el_lowerer_extract_tuple_field(lw, slice, EL_MIR_SLICE_FIELD_DATA);
    }
    case EL_HIR_INTR_MAKE_SLICE: {
        ElMirValue* data = el_lowerer_lower_expr(lw, hir->as.intr.params.rwslice);
        ElMirValue* len = el_lowerer_lower_expr(lw, hir->as.intr.params.len);
        ElMirType* slice_type = el_lowerer_map_type(lw, hir->type);
        ElMirValue* fields[] = { data, len };
        return _el_lowerer_make_tuple(lw, slice_type, fields);
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirIntrKind, hir->as.intr.kind);
}

ElMirValue* el_lowerer_lower_expr(ElLowerer* lw, ElHirExpr* hir) {
    switch (hir->kind) {
    case EL_HIR_EXPR_BINARY:   return _el_lowerer_lower_bin_expr(lw, hir, &hir->as.binary);
    case EL_HIR_EXPR_UNARY:    return _el_lowerer_lower_unary_expr(lw, hir, &hir->as.unary);
    case EL_HIR_EXPR_CALL:     return _el_lowerer_lower_call_expr(lw, hir, &hir->as.call);
    case EL_HIR_EXPR_INTR:     return _el_lowerer_lower_intr_expr(lw, hir);
    case EL_HIR_EXPR_ARRAYLIT: return _el_lowerer_lower_array_lit_expr(lw, hir);
    case EL_HIR_EXPR_CAST:     return _el_lowerer_lower_cast_expr(lw, hir);
    case EL_HIR_EXPR_SYMBOL:   return el_lowerer_lower_symbol(lw, hir->as.symbol, hir->type);
    case EL_HIR_EXPR_TMEMBER: {
        if (el_hir_expr_is_lvalue(hir->as.tmember.expr)) {
            ElMirValue* field_ptr = el_lowerer_get_lvalue(lw, hir);
            ElMirType* field_type = el_lowerer_map_type(lw, hir->type);
            ElMirValue* result = el_mir_new_reg(lw->arena, field_type, lw->current_func->reg_count++);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, result, field_ptr));
            return result;
        } else {
            ElMirValue* val = el_lowerer_lower_expr(lw, hir->as.tmember.expr);
            return _el_lowerer_extract_tuple_field(lw, val, hir->as.tmember.index);
        }
    }
    case EL_HIR_EXPR_MEMBER: {
        if (el_hir_expr_is_lvalue(hir->as.member.expr)) {
            ElMirValue* field_ptr = el_lowerer_get_lvalue(lw, hir);
            ElMirType* field_type = el_lowerer_map_type(lw, hir->type);
            ElMirValue* result = el_mir_new_reg(lw->arena, field_type, lw->current_func->reg_count++);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, result, field_ptr));
            return result;
        } else {
            ElMirValue* val = el_lowerer_lower_expr(lw, hir->as.member.expr);
            return _el_lowerer_extract_tuple_field(lw, val, hir->as.member.index);
        }
    }
    case EL_HIR_EXPR_CONST: {
        ElMirType* mir_type = el_lowerer_map_type(lw, hir->type);
        ElMirConstant mir_const;

        EL_ASSERT(hir->type->kind == EL_HIR_TYPE_PRIM, "constant of non-primitive type");
        if (hir->type->as.prim.kind == EL_PRIMTYPE_INT) {
            mir_const.as.int_ = hir->as.constant.as.int_;
        } else if (hir->type->as.prim.kind == EL_PRIMTYPE_BOOL) {
            mir_const.as.int_ = hir->as.constant.as.bool_ ? 1 : 0;
        } else if (hir->type->as.prim.kind == EL_PRIMTYPE_CHAR) {
            mir_const.as.int_ = (int64_t)hir->as.constant.as.char_;
        } else if (hir->type->as.prim.kind == EL_PRIMTYPE_FLOAT) {
            mir_const.as.float_ = hir->as.constant.as.float_;
        } else {
            EL_UNREACHABLE("invalid hir constant");
        }
        return el_mir_new_const(lw->arena, mir_type, mir_const);
    }
    case EL_HIR_EXPR_UNTYPEDLIT:
        EL_UNREACHABLE("untyped literal in lowerer");
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirExprKind, hir->kind);
}

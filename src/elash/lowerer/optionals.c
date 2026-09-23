#include "lowerer-internals.h"

#include <elash/sema/unary-op.h>
#include <elash/sema/bin-op.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/instr.h>
#include <elash/mir/type.h>
#include <elash/mir/value.h>

#include <elash/sema/values.h>

bool _el_hir_type_opt_is_ref(const ElHirType* type) {
    return type->kind == EL_HIR_TYPE_OPT
        && (type->as.opt.base->kind == EL_HIR_TYPE_REF
        ||  type->as.opt.base->kind == EL_HIR_TYPE_RWSLICE);
}

static ElMirValue* null_ptr(ElLowerer* lw, ElMirType* ptr_type) {
    ElMirConstant zero   = { .kind = EL_MIR_CONST_INT, .as.int_ = EL_INT128(0) };
    ElMirValue* zero_val = el_mir_new_const(lw->arena, lw->tcache->usize_type, zero);
    ElMirValue* result    = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_bitcast_instr(lw->arena, result, zero_val));
    return result;
}

// this approach sucks. we should not be forced to generate a zero value here,
// we'll be able to refactor this once we add something like `undef` expression
static ElMirValue* zero_mir_value(ElLowerer* lw, ElMirType* type) {
    switch (type->kind) {
    case EL_MIR_TYPE_INT: {
        ElMirConstant c = { .kind = EL_MIR_CONST_INT, .as.int_ = EL_INT128(0) };
        return el_mir_new_const(lw->arena, type, c);
    }
    case EL_MIR_TYPE_FLOAT: {
        ElMirConstant c = { .kind = EL_MIR_CONST_FLOAT, .as.float_ = 0.0 };
        return el_mir_new_const(lw->arena, type, c);
    }
    case EL_MIR_TYPE_TUPLE: {
        ElMirValue** fields = EL_DYNARENA_NEW_ARR(lw->arena, ElMirValue*, type->as.tuple.item_count);
        for (usize i = 0; i < type->as.tuple.item_count; ++i) {
            fields[i] = zero_mir_value(lw, type->as.tuple.items[i]);
        }
        return _el_lowerer_make_tuple(lw, type, fields);
    }
    case EL_MIR_TYPE_PTR:
        return null_ptr(lw, type);
    case EL_MIR_TYPE_ARRAY:
    case EL_MIR_TYPE_FUNC:
    case EL_MIR_TYPE_VOID:
        break;
    }
    EL_TODO("yes.");
}

ElMirValue* _el_lowerer_opt_has_value(ElLowerer* lw, const ElHirType* type, ElMirValue* opt) {
    if (_el_hir_type_opt_is_ref(type)) {
        ElMirType* ptr_type = el_tcache_get_mir(lw->tcache, type);
        ElMirValue* null = null_ptr(lw, ptr_type);
        ElMirType* bool_type = lw->tcache->bool_type;
        ElMirValue* result = el_mir_new_reg(lw->arena, bool_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, result, EL_BIN_OP_NEQ, opt, null));
        return result;
    }
    return _el_lowerer_extract_tuple_field(lw, opt, OPT_FIELD_HAS_VALUE);
}

ElMirValue* _el_lowerer_opt_get_value(ElLowerer* lw, const ElHirType* type, ElMirValue* opt) {
    if (_el_hir_type_opt_is_ref(type))
        return opt;
    return _el_lowerer_extract_tuple_field(lw, opt, OPT_FIELD_VALUE);
}

ElMirValue* _el_lowerer_make_null_opt(ElLowerer* lw, const ElHirType* type) {
    if (_el_hir_type_opt_is_ref(type)) {
        return null_ptr(lw, el_tcache_get_mir(lw->tcache, type));
    }

    ElMirType* mir_opt = el_tcache_get_mir(lw->tcache, type);
    ElMirType* base_mir = mir_opt->as.tuple.items[OPT_FIELD_VALUE];
    ElMirConstant false_ = { .kind = EL_MIR_CONST_INT, .as.int_ = EL_INT128(0) };
    ElMirValue* has = el_mir_new_const(lw->arena, lw->tcache->bool_type, false_);
    ElMirValue* val = zero_mir_value(lw, base_mir);
    ElMirValue* fields[] = { has, val };
    return _el_lowerer_make_tuple(lw, mir_opt, fields);
}

ElMirValue* _el_lowerer_make_some_opt(ElLowerer* lw, const ElHirType* type, ElMirValue* value) {
    if (_el_hir_type_opt_is_ref(type)) {
        return value;
    }

    ElMirType* mir_opt = el_tcache_get_mir(lw->tcache, type);
    ElMirConstant true_c = { .kind = EL_MIR_CONST_INT, .as.int_ = EL_INT128(1) };
    ElMirValue* has = el_mir_new_const(lw->arena, lw->tcache->bool_type, true_c);

    ElMirValue** fields = EL_DYNARENA_NEW_ARR(lw->arena, ElMirValue*, 2);
    fields[OPT_FIELD_HAS_VALUE] = has;
    fields[OPT_FIELD_VALUE]     = value;

    return _el_lowerer_make_tuple(lw, mir_opt, fields);
}

ElMirValue* _el_lowerer_get_opt_lvalue(ElLowerer* lw, ElHirExpr* operand) {
    const ElHirType* type = operand->type;
    if (_el_hir_type_opt_is_ref(type)) {
        return el_lower_expr(lw, operand);
    }

    if (el_is_lvalue(operand)) {
        ElMirValue* opt_ptr = el_lowerer_get_lvalue(lw, operand);
        return _el_lowerer_get_tuple_field_ptr(lw, opt_ptr, OPT_FIELD_VALUE);
    }

    ElMirType* mir_opt = el_tcache_get_mir(lw->tcache, type);
    ElMirValue* opt = el_lower_expr(lw, operand);
    ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_opt);
    ElMirValue* tmp = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, tmp, mir_opt));
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, tmp, opt));
    return _el_lowerer_get_tuple_field_ptr(lw, tmp, OPT_FIELD_VALUE);
}

static ElMirValue* lower_conditional_opt(ElLowerer* lw, ElMirType* mir_type, ElMirValue* has, ElMirValue* some_val, ElMirValue* none_val) {
    ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, mir_type);
    ElMirValue* res_ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

    uint32_t some_id = lw->current_func->block_count++;
    uint32_t none_id = lw->current_func->block_count++;
    uint32_t merge_id = lw->current_func->block_count++;

    el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, res_ptr, mir_type));
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, has, some_id, none_id));

    el_lowerer_emit_block(lw, lw->current_block_id);
    lw->current_block_id = none_id;
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, res_ptr, none_val));
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = some_id;
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, res_ptr, some_val));
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, merge_id));
    el_lowerer_emit_block(lw, lw->current_block_id);

    lw->current_block_id = merge_id;
    ElMirValue* result = el_mir_new_reg(lw->arena, mir_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, result, res_ptr));
    return result;
}

ElMirValue* _el_lower_opt_fb(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin) {
    const ElHirType* type = bin->left->type;
    ElMirType* mir_type = el_tcache_get_mir(lw->tcache, hir->type);

    ElMirValue* left = el_lower_expr(lw, bin->left);
    ElMirValue* has = _el_lowerer_opt_has_value(lw, type, left);
    ElMirValue* right = el_lower_expr(lw, bin->right);

    return lower_conditional_opt(lw, mir_type, has, left, right);
}

ElMirValue* _el_lower_opt_map(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin) {
    const ElHirType* type = bin->left->type;
    ElMirType* mir_type = el_tcache_get_mir(lw->tcache, hir->type);

    ElMirValue* left = el_lower_expr(lw, bin->left);
    ElMirValue* has = _el_lowerer_opt_has_value(lw, type, left);
    ElMirValue* mapped = _el_lowerer_make_some_opt(lw, hir->type, el_lower_expr(lw, bin->right));
    ElMirValue* none = _el_lowerer_make_null_opt(lw, hir->type);

    return lower_conditional_opt(lw, mir_type, has, mapped, none);
}

ElMirValue* _el_lower_opt_opt_cmp(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin) {
    (void)hir;

    const ElHirType* type = bin->left->type;
    if (_el_hir_type_opt_is_ref(type)) {
        ElMirValue* lhs = el_lower_expr(lw, bin->left);
        ElMirValue* other = el_lower_expr(lw, bin->right);
        ElMirValue* result = el_mir_new_reg(lw->arena, lw->tcache->bool_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, result, bin->op, lhs, other));
        return result;
    }

    ElMirValue* left      = el_lower_expr(lw, bin->left);
    ElMirValue* right     = el_lower_expr(lw, bin->right);
    ElMirValue* left_has  = _el_lowerer_opt_has_value(lw, type, left);
    ElMirValue* right_has = _el_lowerer_opt_has_value(lw, type, right);
    ElMirValue* left_val  = _el_lowerer_opt_get_value(lw, type, left);
    ElMirValue* right_val = _el_lowerer_opt_get_value(lw, type, right);

    // the "algorithm" is roughly:
    // both-some = left-has && right-has
    // both-null = !left-has && !right-has
    // vals-eq   = left-value == right-value
    // some-eq   = both-some && vals-eq
    // is-equal  = both-null || some-eq

    ElMirValue* left_empty  = emit_unary(lw, EL_UNARY_OP_NOT, left_has);
    ElMirValue* right_empty = emit_unary(lw, EL_UNARY_OP_NOT, right_has);
    ElMirValue* both_null   = emit_bin(lw, EL_BIN_OP_AND, left_empty, right_empty);

    ElMirValue* both_some = emit_bin(lw, EL_BIN_OP_AND, left_has, right_has);
    ElMirValue* vals_eq   = emit_bin(lw, EL_BIN_OP_EQ,  left_val, right_val);
    ElMirValue* some_eq   = emit_bin(lw, EL_BIN_OP_AND, both_some, vals_eq);
    ElMirValue* is_eq     = emit_bin(lw, EL_BIN_OP_OR,  both_null, some_eq);

    if (bin->op == EL_BIN_OP_EQ) {
        return is_eq;
    } else {
        // Theoretically, we could optimize it a bit. Instead of using == and then negating the entire result, we
        // count generate expressions like `different-nullness || vals-neq`, BUT we're using llvm and llvm optimization
        // passes are clearly smarter than me. let's just let them do their job. i LOVE llvm (it's slow, though).
        ElMirValue* result = el_mir_new_reg(lw->arena, lw->tcache->bool_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_unary_instr(lw->arena, result, EL_UNARY_OP_NOT, is_eq));
        return result;
    }
}

ElMirValue* _el_lower_opt_base_cmp(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin) {
    // TODO: this generates ugly MIR
    (void)hir;

    const ElHirType* type = bin->left->type;
    ElMirType* bool_type = lw->tcache->bool_type;

    ElMirValue* opt   = el_lower_expr(lw, bin->left);
    ElMirValue* other = el_lower_expr(lw, bin->right);

    ElMirValue* has = _el_lowerer_opt_has_value(lw, type, opt);
    ElMirValue* val = _el_lowerer_opt_get_value(lw, type, opt);

    ElMirValue* val_cmp = el_mir_new_reg(lw->arena, bool_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, val_cmp, bin->op, val, other));

    ElMirValue* result = el_mir_new_reg(lw->arena, bool_type, lw->current_func->reg_count++);
    if (bin->op == EL_BIN_OP_EQ) {
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, result, EL_BIN_OP_AND, has, val_cmp));
    } else if (bin->op == EL_BIN_OP_NEQ) {
        ElMirValue* not_has = el_mir_new_reg(lw->arena, bool_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_unary_instr(lw->arena, not_has, EL_UNARY_OP_NOT, has));
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, result, EL_BIN_OP_OR, not_has, val_cmp));
    } else {
        EL_UNREACHABLE("unsupported optional comparison");
    }

    return result;
}

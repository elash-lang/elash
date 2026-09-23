#pragma once
#include <elash/lowerer/lowerer.h>  // IWYU pragma: export

enum {
    EL_MIR_SLICE_FIELD_DATA = 0,
    EL_MIR_SLICE_FIELD_LEN  = 1,
};
enum {
    OPT_FIELD_HAS_VALUE = 0,
    OPT_FIELD_VALUE     = 1,
};

ElMirValue*    _el_lowerer_extract_tuple_field(ElLowerer* lw, ElMirValue* tuple, usize index);
ElMirValue*    _el_lowerer_get_tuple_field_ptr(ElLowerer* lw, ElMirValue* tuple_ptr, usize index);
ElMirValue*    _el_lowerer_make_tuple(ElLowerer* lw, ElMirType* tuple_type, ElMirValue** fields);
ElMirConstant* _el_lower_const(ElLowerer* lw, ElHirExpr* expr);
ElMirValue*    _el_lowerer_new_anon_global(ElLowerer* lw, ElMirType* type, ElMirConstant* init);
ElMirValue*    _el_lowerer_create_alloca(ElLowerer* lw, ElMirType* type);
void           _el_lowerer_copy_str_to_ptr(ElLowerer* lw, ElMirValue* ptr, ElMirStrConst str);
void           _el_lower_agginit(ElLowerer* lw, ElMirValue* ptr, ElHirAggInit* agginit);

usize _el_lowerer_sizeof(ElLowerer* lw, ElMirType* type);
usize _el_lowerer_alignof(ElLowerer* lw, ElMirType* type);

//////////////////// optionals /////////////////////
bool        _el_hir_type_opt_is_ref(const ElHirType* type);
ElMirValue* _el_lowerer_opt_has_value(ElLowerer* lw, const ElHirType* type, ElMirValue* opt);
ElMirValue* _el_lowerer_opt_get_value(ElLowerer* lw, const ElHirType* type, ElMirValue* opt);
ElMirValue* _el_lowerer_make_some_opt(ElLowerer* lw, const ElHirType* type, ElMirValue* value);
ElMirValue* _el_lowerer_make_null_opt(ElLowerer* lw, const ElHirType* type);

ElMirValue* _el_lowerer_get_opt_lvalue(ElLowerer* lw, ElHirExpr* operand);
ElMirValue* _el_lower_opt_fb(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin);
ElMirValue* _el_lower_opt_map(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin);
ElMirValue* _el_lower_opt_base_cmp(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin);
ElMirValue* _el_lower_opt_opt_cmp(ElLowerer* lw, ElHirExpr* hir, ElHirBinExpr* bin);

///////////////// helpers /////////////////////
static inline ElMirValue* emit_bin(ElLowerer* lw, ElBinOp op, ElMirValue* lhs, ElMirValue* rhs) {
    ElMirValue* res =el_mir_new_reg(lw->arena, lw->tcache->bool_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_bin_instr(lw->arena, res, op, lhs, rhs));
    return res;
}

static inline ElMirValue* emit_unary(ElLowerer* lw, ElUnaryOp op, ElMirValue* val) {
    ElMirValue* res = el_mir_new_reg(lw->arena, lw->tcache->bool_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_unary_instr(lw->arena, res, op, val));
    return res;
}

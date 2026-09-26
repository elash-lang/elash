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

/////////////////// tuples ///////////////////
ElMirValue*    _el_lowerer_extract_tuple_field(ElLowerer* lw, ElMirValue* tuple, usize index);
ElMirValue*    _el_lowerer_get_tuple_field_ptr(ElLowerer* lw, ElMirValue* tuple_ptr, usize index);
ElMirValue*    _el_lowerer_make_tuple(ElLowerer* lw, ElMirType* tuple_type, ElMirValue** fields);

ElMirConstant* _el_lower_const(ElLowerer* lw, ElHirExpr* expr);

////////////////// globals ////////////////////
ElMirValue*    _el_lowerer_get_symbol_lvalue(ElLowerer* lw, ElHirSymbol* sym, const ElHirType* type);
ElMirValue*    _el_lowerer_new_anon_global(
    ElLowerer* lw, ElMirType* type, ElMirConstant* init, bool is_constant
);

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
ElMirValue*    _el_lowerer_create_alloca(ElLowerer* lw, ElMirType* type);
void           _el_lowerer_copy_str_to_ptr(ElLowerer* lw, ElMirValue* ptr, ElMirStrConst str);
void           _el_lower_agginit(ElLowerer* lw, ElMirValue* ptr, ElHirAggInit* agginit);

usize _el_lowerer_sizeof(ElLowerer* lw, ElMirType* type);
usize _el_lowerer_alignof(ElLowerer* lw, ElMirType* type);

static inline ElMirValue* emit_reg(ElLowerer* lw, ElMirType* type) {
    return el_mir_new_reg(lw->arena, type, lw->current_func->reg_count++);
}

static inline uint32_t new_block_id(ElLowerer* lw) {
    return lw->current_func->block_count++;
}

static inline ElMirValue* emit_load(ElLowerer* lw, ElMirType* type, ElMirValue* ptr) {
    ElMirValue* reg = emit_reg(lw, type);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, reg, ptr));
    return reg;
}
static inline void emit_store(ElLowerer* lw, ElMirValue* ptr, ElMirValue* val) {
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, ptr, val));
}
static inline void emit_jmp(ElLowerer* lw, uint32_t target_id) {
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmp_instr(lw->arena, target_id));
}
static inline void emit_jmpif(ElLowerer* lw, ElMirValue* cond, uint32_t then_id, uint32_t else_id) {
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_jmpif_instr(lw->arena, cond, then_id, else_id));
}

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

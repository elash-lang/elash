#include <elash/lowerer/lowerer.h>
#include <elash/mir/instr.h>
#include <elash/mir/type.h>

#include <elash/util/assert.h>

ElMirValue* _el_lowerer_get_tuple_field_ptr(ElLowerer* lw, ElMirValue* tuple_ptr, usize index) {
    EL_ASSERT(tuple_ptr->type->kind == EL_MIR_TYPE_PTR, "expected tuple pointer");
    ElMirType* tuple_type = tuple_ptr->type->as.ptr.base;
    EL_ASSERT(tuple_type->kind == EL_MIR_TYPE_TUPLE, "expected tuple type");
    EL_ASSERT(index < tuple_type->as.tuple.item_count, "tuple field index out of range");

    ElMirType* field_type = tuple_type->as.tuple.items[index];
    ElMirType* field_ptr_type = el_mir_new_ptr_type(lw->arena, field_type);
    ElMirValue* field_ptr = el_mir_new_reg(lw->arena, field_ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_gfp_instr(lw->arena, field_ptr, tuple_ptr, index));
    return field_ptr;
}

ElMirValue* _el_lowerer_extract_tuple_field(ElLowerer* lw, ElMirValue* tuple, usize index) {
    EL_ASSERT(tuple->type->kind == EL_MIR_TYPE_TUPLE, "extract tuple field called on non-tuple value");

    ElMirType* tuple_ptr_type = el_mir_new_ptr_type(lw->arena, tuple->type);
    ElMirValue* tmp = el_mir_new_reg(lw->arena, tuple_ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, tmp, tuple->type));
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, tmp, tuple));

    ElMirValue* field_ptr = _el_lowerer_get_tuple_field_ptr(lw, tmp, index);

    ElMirType* field_type = tuple->type->as.tuple.items[index];
    ElMirValue* result = el_mir_new_reg(lw->arena, field_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, result, field_ptr));
    return result;
}

ElMirValue* _el_lowerer_make_tuple(ElLowerer* lw, ElMirType* tuple_type, ElMirValue** fields) {
    EL_ASSERT(tuple_type->kind == EL_MIR_TYPE_TUPLE, "expected tuple type");

    ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, tuple_type);
    ElMirValue* tmp = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_alloca_instr(lw->arena, tmp, tuple_type));

    for (usize i = 0; i < tuple_type->as.tuple.item_count; ++i) {
        ElMirType* field_ptr_type = el_mir_new_ptr_type(lw->arena, tuple_type->as.tuple.items[i]);
        ElMirValue* field_ptr = el_mir_new_reg(lw->arena, field_ptr_type, lw->current_func->reg_count++);
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_gfp_instr(lw->arena, field_ptr, tmp, i));
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, field_ptr, fields[i]));
    }

    ElMirValue* result = el_mir_new_reg(lw->arena, tuple_type, lw->current_func->reg_count++);
    el_mir_ibuf_push(&lw->ibuf, el_mir_new_load_instr(lw->arena, result, tmp));
    return result;
}


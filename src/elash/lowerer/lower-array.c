#include <elash/lowerer/lowerer.h>
#include <elash/mir/instr.h>

void _el_lowerer_lower_array_lit(ElLowerer* lw, ElMirValue* ref, ElHirArrayLit* array_lit) {
    for (usize i = 0; i < array_lit->count; ++i) {
        ElConstant idx_lit = { .as.int_ = (int64_t)i };
        ElMirValue* index = el_mir_new_const(lw->arena, lw->builtins->type_int, idx_lit);

        ElType* elem_type = array_lit->values[i]->type;
        ElType* ref_type = el_sema_new_ref_type(lw->arena, elem_type);
        ElMirValue* elem_ref = el_mir_new_reg(lw->arena, ref_type, lw->current_func->reg_count++);

        el_mir_ibuf_push(&lw->ibuf, el_mir_new_gep_instr(lw->arena, elem_ref, ref, index));

        if (array_lit->values[i]->kind == EL_HIR_EXPR_ARRAYLIT) {
            _el_lowerer_lower_array_lit(lw, elem_ref, &array_lit->values[i]->as.array_lit);
        } else {
            ElMirValue* val = el_lowerer_lower_expr(lw, array_lit->values[i]);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, elem_ref, val));
        }
    }
}

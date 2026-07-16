#include <elash/lowerer/lowerer.h>
#include <elash/lowerer/builtin.h>

#include <elash/mir/value/const.h>
#include <elash/mir/value/reg.h>
#include <elash/mir/instr/gep.h>
#include <elash/mir/instr.h>
#include <elash/mir/type.h>

void _el_lowerer_lower_array_lit(ElLowerer* lw, ElMirValue* ptr, ElHirArrayLit* array_lit) {
    for (usize i = 0; i < array_lit->count; ++i) {
        ElMirConstant idx_lit = { .as.int_ = (int64_t)i };
        ElMirValue* index = el_mir_new_const(lw->arena, lw->builtins->type_usize, idx_lit);

        ElMirType* elem_type = el_lowerer_map_type(lw, array_lit->values[i]->type);
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, elem_type);
        ElMirValue* elem_ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

        el_mir_ibuf_push(&lw->ibuf, el_mir_new_gep_instr(lw->arena, elem_ptr, ptr, index));

        if (array_lit->values[i]->kind == EL_HIR_EXPR_ARRAYLIT) {
            _el_lowerer_lower_array_lit(lw, elem_ptr, &array_lit->values[i]->as.array_lit);
        } else {
            ElMirValue* val = el_lowerer_lower_expr(lw, array_lit->values[i]);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, elem_ptr, val));
        }
    }
}

#include <elash/lowerer/lowerer.h>
#include <elash/mir/instr.h>

void _el_lowerer_lower_array_lit(ElLowerer* lw, ElMirValue* ptr, ElHirArrayLitNode* array_lit) {
    for (usize i = 0; i < array_lit->count; ++i) {
        ElHirLiteral idx_lit = { .as.int_ = (int64_t)i };
        ElMirValue* index = el_mir_new_const(lw->arena, lw->builtins->type_int, idx_lit);
        
        ElType* elem_type = array_lit->values[i]->type;
        ElType* ptr_type = el_sema_new_ptr_type(lw->arena, elem_type);
        ElMirValue* elem_ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);
        
        el_mir_ibuf_push(&lw->ibuf, el_mir_new_gep_instr(lw->arena, elem_ptr, ptr, index));
        
        if (array_lit->values[i]->kind == EL_HIR_EXPR_ARRAY_LITERAL) {
            _el_lowerer_lower_array_lit(lw, elem_ptr, &array_lit->values[i]->as.array_lit);
        } else {
            ElMirValue* val = el_lowerer_lower_expr(lw, array_lit->values[i]);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, elem_ptr, val));
        }
    }
}


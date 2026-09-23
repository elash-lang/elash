#include "lowerer-internals.h"

#include <elash/util/assert.h>

#include <elash/mir/value.h>
#include <elash/mir/instr.h>
#include <elash/mir/type.h>

void _el_lower_agginit(ElLowerer* lw, ElMirValue* ptr, ElHirAggInit* agginit) {
    EL_ASSERT(ptr->type->kind == EL_MIR_TYPE_PTR, "expected pointer for agginit target");
    ElMirType* target_type = ptr->type->as.ptr.base;

    for (usize i = 0; i < agginit->count; ++i) {
        ElMirType* elem_type = el_tcache_get_mir(lw->tcache, agginit->values[i]->type);
        ElMirType* ptr_type = el_mir_new_ptr_type(lw->arena, elem_type);
        ElMirValue* elem_ptr = el_mir_new_reg(lw->arena, ptr_type, lw->current_func->reg_count++);

        if (target_type->kind == EL_MIR_TYPE_ARRAY) {
            ElMirConstant idx_lit = { .kind = EL_MIR_CONST_INT, .as.int_ = EL_INT128((int64_t)i) };
            ElMirValue* index = el_mir_new_const(lw->arena, lw->builtins->type_usize, idx_lit);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_gep_instr(lw->arena, elem_ptr, ptr, index));
        } else if (target_type->kind == EL_MIR_TYPE_TUPLE) {
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_gfp_instr(lw->arena, elem_ptr, ptr, i));
        } else {
            EL_UNREACHABLE("unexpected target type for agginit lowering");
        }

        if (agginit->values[i]->kind == EL_HIR_EXPR_AGGINIT) {
            _el_lower_agginit(lw, elem_ptr, &agginit->values[i]->as.agginit);
        } else {
            ElMirValue* val = el_lower_expr(lw, agginit->values[i]);
            el_mir_ibuf_push(&lw->ibuf, el_mir_new_store_instr(lw->arena, elem_ptr, val));
        }
    }
}

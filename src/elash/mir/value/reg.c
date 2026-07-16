#include <elash/mir/value/reg.h>
#include <elash/mir/value.h>

ElMirValue* el_mir_new_reg(ElDynArena* arena, ElMirType* type, uint32_t reg_id) {
    ElMirValue* val = EL_DYNARENA_NEW(arena, ElMirValue);
    val->kind = EL_MIR_VAL_REG;
    val->type = type;
    val->as.reg.id = reg_id;
    return val;
}

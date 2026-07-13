#include <elash/mir/value/const.h>
#include <elash/mir/value.h>

ElMirValue* el_mir_new_const(ElDynArena* arena, ElType* type, ElConstant constant) {
    ElMirValue* val = EL_DYNARENA_NEW(arena, ElMirValue);
    val->kind = EL_MIR_VAL_CONST;
    val->type = type;
    val->as.constant = constant;
    return val;
}

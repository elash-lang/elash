#include <elash/mir/value/arg.h>
#include <elash/mir/value.h>

ElMirValue* el_mir_new_arg(ElDynArena* arena, ElMirType* type, uint32_t arg_idx) {
    ElMirValue* val = EL_DYNARENA_NEW(arena, ElMirValue);
    val->kind = EL_MIR_VAL_ARG;
    val->type = type;
    val->as.arg.idx = arg_idx;
    return val;
}

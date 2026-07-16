#include <elash/mir/value/global.h>
#include <elash/mir/value.h>

ElMirValue* el_mir_new_global(ElDynArena* arena, ElMirType* type, ElMirSymbol* global) {
    ElMirValue* val = EL_DYNARENA_NEW(arena, ElMirValue);
    val->kind = EL_MIR_VAL_GLOBAL;
    val->type = type;
    val->as.global.sym = global;
    return val;
}

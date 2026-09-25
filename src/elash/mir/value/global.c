#include <elash/mir/value/global.h>
#include <elash/mir/value.h>

ElMirValue* el_mir_new_global(
    ElDynArena* arena, ElMirType* type,
    ElMirSymbol* global, ElMirConstant* init, bool is_definition, bool is_constant
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirValue, {
        .kind = EL_MIR_VAL_GLOBAL,
        .type = type,
        .as.global = {
            .sym = global, .init = init,
            .is_definition = is_definition,
            .is_constant = is_constant,
        },
    });
}

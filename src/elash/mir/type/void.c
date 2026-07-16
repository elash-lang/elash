#include <elash/mir/type.h>

ElMirType* el_mir_new_void_type(ElDynArena* arena) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_VOID,
    });
}

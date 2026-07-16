#include <elash/mir/type.h>

ElMirType* el_mir_new_array_type(ElDynArena* arena, ElMirType* base, usize size) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_ARRAY,
        .as.array = {
            .base = base,
            .size = size,
        },
    });
}

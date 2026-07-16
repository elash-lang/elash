#include <elash/mir/type.h>

ElMirType* el_mir_new_ptr_type(ElDynArena* arena, ElMirType* base) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_PTR,
        .as.ptr = {
            .base = base,
        },
    });
}

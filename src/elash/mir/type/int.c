#include <elash/mir/type.h>

ElMirType* el_mir_new_int_type(ElDynArena* arena, uint32_t width, bool is_signed) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_INT,
        .as.integer = {
            .width = width,
            .is_signed = is_signed,
        },
    });
}

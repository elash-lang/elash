#include <elash/mir/type.h>
#include <elash/mir/type/float.h>

ElMirType* el_mir_new_float_type(ElDynArena* arena, uint32_t width) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_FLOAT,
        .as.float_.width = width,
    });
}

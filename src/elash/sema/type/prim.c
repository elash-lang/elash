#include <elash/sema/type/prim.h>
#include <elash/sema/type.h>

ElType* el_sema_new_int_type(ElDynArena* arena, ElBasicIntWidth width, bool is_signed) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElType, {
        .kind = EL_TYPE_PRIM,
        .as.prim = {
            .kind = EL_PRIMTYPE_INT,
            .as.integral = {
                width, is_signed,
            },
        },
    });
}

ElType* el_sema_new_prim_type(ElDynArena* arena, ElPrimitiveTypeKind kind) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElType, {
        .kind = EL_TYPE_PRIM,
        .as.prim.kind = kind,
    });
}

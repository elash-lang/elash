#include <elash/hir/type/prim.h>
#include <elash/hir/type.h>

ElHirType* el_hir_new_int_type(ElDynArena* arena, ElHirIntWidth width, bool is_signed) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirType, {
        .kind = EL_HIR_TYPE_PRIM,
        .as.prim = {
            .kind = EL_PRIMTYPE_INT,
            .as.integral = {
                width, is_signed,
            },
        },
    });
}

ElHirType* el_hir_new_prim_type(ElDynArena* arena, ElHirPrimTypeKind kind) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirType, {
        .kind = EL_HIR_TYPE_PRIM,
        .as.prim.kind = kind,
    });
}

#include <elash/hir/type.h>

ElHirType* el_hir_new_tuple_type(ElDynArena* arena, ElHirType** elements, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirType, {
        .kind = EL_HIR_TYPE_TUPLE,
        .as.tuple = { elements, count },
    });
}

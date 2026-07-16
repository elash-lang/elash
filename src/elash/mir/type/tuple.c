#include <elash/mir/type.h>

ElMirType* el_mir_new_tuple_type(ElDynArena* arena, ElMirType** items, usize item_count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_TUPLE,
        .as.tuple = {
            .items = items,
            .item_count = item_count,
        },
    });
}


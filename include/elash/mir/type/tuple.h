#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/dynarena.h>

typedef struct ElMirType ElMirType;
typedef struct ElMirTupleType {
    ElMirType** items;
    usize item_count;
} ElMirTupleType;

ElMirType* el_mir_new_tuple_type(ElDynArena* arena, ElMirType** items, usize item_count);


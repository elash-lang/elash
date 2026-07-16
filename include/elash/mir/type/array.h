#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/dynarena.h>

typedef struct ElMirType ElMirType;
typedef struct ElMirArrayType {
    ElMirType* base;
    usize size;
} ElMirArrayType;

ElMirType* el_mir_new_array_type(ElDynArena* arena, ElMirType* base, usize size);

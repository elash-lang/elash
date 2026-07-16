#pragma once

#include <elash/util/dynarena.h>

typedef struct ElMirType ElMirType;
typedef struct ElMirPtrType {
    ElMirType* base;
} ElMirPtrType;

ElMirType* el_mir_new_ptr_type(ElDynArena* arena, ElMirType* base);

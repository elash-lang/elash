#pragma once

#include <elash/util/dynarena.h>

#include <stdint.h>

typedef struct ElMirType ElMirType;
typedef struct ElMirFloatType {
    uint32_t width;
} ElMirFloatType;

ElMirType* el_mir_new_float_type(ElDynArena* arena, uint32_t width);

#pragma once

#include <elash/util/dynarena.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct ElMirType ElMirType;
typedef struct ElMirIntType {
    uint32_t width;
    bool is_signed;
} ElMirIntType;

ElMirType* el_mir_new_int_type(ElDynArena* arena, uint32_t width, bool is_signed);

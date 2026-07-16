#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/type.h>

#include <stdint.h>

typedef struct ElMirValue ElMirValue;

typedef struct ElMirArgValue {
    uint32_t idx;
} ElMirArgValue;

ElMirValue* el_mir_new_arg(ElDynArena* arena, ElMirType* type, uint32_t arg_idx);

#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/type.h>

#include <stdint.h>

typedef struct ElMirValue ElMirValue;

typedef struct ElMirRegValue {
    uint32_t id;
} ElMirRegValue;

ElMirValue* el_mir_new_reg(ElDynArena* arena, ElMirType* type, uint32_t reg_id);

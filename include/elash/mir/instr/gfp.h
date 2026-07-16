#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirGfpInstr {
    ElMirValue* ptr;
    usize index;
} ElMirGfpInstr;

ElMirInstr* el_mir_new_gfp_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ptr, usize index);

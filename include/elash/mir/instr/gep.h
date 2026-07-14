#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirGepInstr {
    ElMirValue* ref;
    ElMirValue* index;
} ElMirGepInstr;

ElMirInstr* el_mir_new_gep_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ref, ElMirValue* index);

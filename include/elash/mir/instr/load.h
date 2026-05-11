#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirLoadInstr {
    ElMirValue* ptr;
} ElMirLoadInstr;

ElMirInstr* el_mir_new_load_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ptr);

#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirAllocaInstr {
    ElType* type;
} ElMirAllocaInstr;

ElMirInstr* el_mir_new_alloca_instr(ElDynArena* arena, ElMirValue* result, ElType* type);

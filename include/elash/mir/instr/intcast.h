#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirIntCastInstr {
    ElMirValue* operand;
} ElMirIntCastInstr;

ElMirInstr* el_mir_new_intcast_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* operand);

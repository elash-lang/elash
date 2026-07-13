#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirBitCastInstr {
    ElMirValue* operand;
} ElMirBitCastInstr;

ElMirInstr* el_mir_new_bitcast_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* operand);

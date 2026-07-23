#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;
typedef struct ElMirFpCastInstr {
    ElMirValue* operand;
} ElMirFpCastInstr;

ElMirInstr* el_mir_new_fpcast_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* operand);

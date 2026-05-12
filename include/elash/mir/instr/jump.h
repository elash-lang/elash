#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirJmpInstr {
    uint32_t target_id;
} ElMirJmpInstr;

typedef struct ElMirJmpIfInstr {
    ElMirValue* cond;
    uint32_t then_id;
    uint32_t else_id;
} ElMirJmpIfInstr;

ElMirInstr* el_mir_new_jmp_instr(ElDynArena* arena, uint32_t target_id);
ElMirInstr* el_mir_new_jmpif_instr(ElDynArena* arena, ElMirValue* cond, uint32_t then_id, uint32_t else_id);

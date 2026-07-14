#pragma once

#include <elash/mir/value.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirStoreInstr {
    ElMirValue* ref;
    ElMirValue* value;
} ElMirStoreInstr;

ElMirInstr* el_mir_new_store_instr(ElDynArena* arena, ElMirValue* ref, ElMirValue* value);

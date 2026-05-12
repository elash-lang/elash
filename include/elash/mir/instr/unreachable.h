#pragma once

#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

ElMirInstr* el_mir_new_unreachable_instr(ElDynArena* arena);

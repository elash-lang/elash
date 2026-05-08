#pragma once

#include "func.h"
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElMirModule {
    ElMirFunc* first_func;
    ElMirFunc* last_func;
    usize func_count;
} ElMirModule;

ElMirModule* el_mir_new_module(ElDynArena* arena);
void el_mir_module_add_func(ElMirModule* mod, ElMirFunc* func);

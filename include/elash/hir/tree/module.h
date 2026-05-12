#pragma once

#include "toplevel.h"
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElHirModule {
    ElHirTopLevelNode* head;
    ElHirTopLevelNode* tail;
    usize count;
    uint32_t sym_count;
} ElHirModule;

ElHirModule* el_hir_new_module(ElDynArena* arena);
void el_hir_module_append(ElHirModule* mod, ElHirTopLevelNode* node);

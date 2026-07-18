#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

#include "decl.h"

typedef struct ElHirModule {
    ElHirDecl* head;
    ElHirDecl* tail;
    usize count;
    uint32_t sym_count;
} ElHirModule;

ElHirModule* el_hir_new_module(ElDynArena* arena);
void el_hir_module_append(ElHirModule* mod, ElHirDecl* node);

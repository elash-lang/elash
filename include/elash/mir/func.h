#pragma once

#include <elash/mir/symbol.h>
#include "block.h"
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElMirFunc ElMirFunc;
struct ElMirFunc {
    ElMirFunc* next;

    ElMirSymbol* symbol;
    ElMirBlock* first_block;
    ElMirBlock* last_block;

    ElMirValue** args;
    uint32_t arg_count;

    uint32_t reg_count;
    uint32_t block_count;
};

ElMirFunc* el_mir_new_func(ElDynArena* arena, ElMirSymbol* symbol);
void el_mir_func_append_block(ElMirFunc* func, ElMirBlock* block);

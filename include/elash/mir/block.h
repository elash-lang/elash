#pragma once

#include <elash/mir/instr-buf.h>
#include <elash/mir/instr.h>

#include <elash/util/dynarena.h>

typedef struct ElMirBlock ElMirBlock;
struct ElMirBlock {
    ElMirBlock* next;
    uint32_t id;
    usize instr_count;
    ElMirInstr* instructions[];
};

ElMirBlock* el_mir_new_block(ElDynArena* arena, uint32_t id, ElMirInstr** instructions, usize instr_count);
ElMirBlock* el_mir_new_block_from_ibuf(ElDynArena* arena, uint32_t id, const ElMirInstrBuf* buf);


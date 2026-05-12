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

static inline ElMirInstr* el_mir_block_get_terminator(ElMirBlock* block) {
    if (block->instr_count == 0) return NULL;
    ElMirInstr* last = block->instructions[block->instr_count - 1];
    return el_mir_instr_is_terminator(last) ? last : NULL;
}


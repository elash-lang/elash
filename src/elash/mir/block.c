#include <elash/mir/block.h>
#include <string.h>

ElMirBlock* el_mir_new_block(ElDynArena* arena, ElMirInstr** instructions, usize instr_count) {
    usize size = sizeof(ElMirBlock) + (sizeof(ElMirInstr*) * instr_count);
    ElMirBlock* block = el_dynarena_alloc_zeroed(arena, size, alignof(ElMirBlock));
    if (block != NULL) {
        block->next = NULL;
        block->instr_count = instr_count;
    
        if (instructions != NULL && instr_count > 0) {
            memcpy(block->instructions, instructions, sizeof(ElMirInstr*) * instr_count);
        }
    }
    return block;
}

ElMirBlock* el_mir_new_block_from_ibuf(ElDynArena* arena, const ElMirInstrBuf* buf) {
    return el_mir_new_block(arena, buf->items, buf->len);
}

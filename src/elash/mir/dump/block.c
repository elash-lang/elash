#include <elash/mir/dump/block.h>
#include <elash/mir/dump/instr.h>
#include <inttypes.h>

void el_mir_dump_block(const ElMirBlock* block, usize indent, FILE* out) {
    if (!block) return;

    for (usize i = 0; i < indent; ++i) fputs(" ", out);
    fprintf(out, "@%"PRIu32":\n", block->id);

    for (usize i = 0; i < block->instr_count; ++i) {
        // Block instructions are indented by 4 spaces.
        el_mir_dump_instr(block->instructions[i], indent + 4, out);
    }
}

#include <elash/mir/dump/func.h>
#include <elash/mir/dump/block.h>
#include <elash/mir/dump/value.h>
#include <elash/sema/symbol/dump.h>
#include <elash/sema/type.h>
#include <elash/defs/sv.h>

#include <inttypes.h>

void el_mir_dump_func(const ElMirFunc* func, usize indent, FILE* out) {
    if (!func) return;

    for (usize i = 0; i < indent; ++i) fputs(" ", out);
    
    fputs("define ", out);
    el_sema_dump_type(func->symbol->as.func.ret_type, out);
    fputc(' ', out);
    fprintf(out, EL_SV_FMT "(", EL_SV_FARG(func->symbol->name));

    for (uint32_t i = 0; i < func->arg_count; ++i) {
        if (i > 0) fputs(", ", out);
        el_mir_dump_value(func->args[i], out);
    }
    fputs(") {\n", out);

    for (ElMirBlock* block = func->first_block; block; block = block->next) {
        el_mir_dump_block(block, indent, out);
    }

    for (usize i = 0; i < indent; ++i) fputs(" ", out);
    fputs("}\n", out);
}

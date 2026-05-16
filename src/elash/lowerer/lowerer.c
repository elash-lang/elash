#include <elash/lowerer/lowerer.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/mir/module.h>
#include <elash/mir/func.h>
#include <elash/mir/block.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>
#include <elash/mir/instr-buf.h>
#include <elash/mir/value/const.h>
#include <elash/mir/value/reg.h>
#include <elash/mir/instr/binary.h>
#include <elash/mir/instr/return.h>

#include <elash/sema/type/prim.h>
#include <elash/sema/symbol/func.h>
#include <elash/sema/symbol/var.h>
#include <elash/sema/expr/bin-op.h>

#include <elash/hir/tree/stmt/assign.h>
#include <elash/mir/value/global.h>

#include <stddef.h>

void el_lowerer_init(ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag, ElBuiltins* builtins) {
    lw->arena = arena;
    lw->diag = diag;
    lw->builtins = builtins;
    el_mir_ibuf_init(&lw->ibuf);
    lw->symbol_map = NULL;
    lw->break_target_id = 0;
    lw->continue_target_id = 0;
}

void el_lowerer_free(ElLowerer* lw) {
    el_mir_ibuf_destroy(&lw->ibuf);
}

bool el_lowerer_has_terminator(ElLowerer* lw) {
    if (lw->ibuf.len == 0) return false;
    return el_mir_instr_is_terminator(lw->ibuf.items[lw->ibuf.len - 1]);
}

void el_lowerer_emit_block(ElLowerer* lw, uint32_t id) {
    if (lw->ibuf.len == 0) return;
    ElMirBlock* block = el_mir_new_block_from_ibuf(lw->arena, id, &lw->ibuf);
    el_mir_func_append_block(lw->current_func, block);
    el_mir_ibuf_clear(&lw->ibuf);
}



#include <elash/lowerer/lowerer.h>

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
#include <elash/sema/expr/bin-op.h>

#include <stddef.h>

// TODO: implementation

void el_lowerer_init(ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag) {
    lw->arena = arena;
    lw->diag = diag;
}

void el_lowerer_free(ElLowerer* lw) {(void) lw;}

ElMirValue*  el_lowerer_lower_expr(ElLowerer* lw, ElHirExprNode* hir) {(void) lw, (void) hir; return NULL; }
void         el_lowerer_lower_stmt(ElLowerer* lw, ElHirStmtNode* hir) {(void) lw, (void) hir;}

void el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir) {
    switch (hir->kind) {
    case EL_HIR_TOPLVL_FUNC_DEF: {
        ElMirFunc* func = el_mir_new_func(lw->arena, hir->as.func_def.symbol);
        el_mir_module_add_func(lw->current_mod, func);
    }
    }
}

// TODO: stub
ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir) { 
    lw->current_mod = el_mir_new_module(lw->arena);
    for (ElHirTopLevelNode* node = hir->head; node != NULL; node = node->next) {
        el_lowerer_lower_toplvl(lw, node);
    }
   
    return lw->current_mod;
}

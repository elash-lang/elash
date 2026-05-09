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
void         el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir) {(void) lw, (void) hir;}

// TODO: stub
ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir) { 
    (void) hir; 

    ElMirModule* mod = el_mir_new_module(lw->arena);
    ElType* int_type = el_sema_new_prim_type(lw->arena, EL_PRIMTYPE_INT);

    ElSymbol* main_sym = el_sema_new_func_symbol(lw->arena, EL_SV("main"), int_type, NULL, 0);
    ElMirFunc* main_func = el_mir_new_func(lw->arena, main_sym);
    
    ElMirInstrBuf ibuf;
    el_mir_ibuf_init(&ibuf);
    
    ElMirValue* c37 = el_mir_new_const(lw->arena, int_type, (ElHirLiteral){ .as.int_ = 37 });
    ElMirValue* c10 = el_mir_new_const(lw->arena, int_type, (ElHirLiteral){ .as.int_ = 10 });
    ElMirValue* res = el_mir_new_reg(lw->arena, int_type, main_func->next_reg_id++);
    
    el_mir_ibuf_push(&ibuf, el_mir_new_bin_instr(lw->arena, res, EL_SEMA_BIN_OP_ADD, c37, c10));
    el_mir_ibuf_push(&ibuf, el_mir_new_ret_instr(lw->arena, res));
 
    ElMirBlock* block = el_mir_new_block_from_ibuf(lw->arena, &ibuf);
    el_mir_func_append_block(main_func, block);
    el_mir_module_add_func(mod, main_func);
    el_mir_ibuf_destroy(&ibuf);
    
    return mod;
}

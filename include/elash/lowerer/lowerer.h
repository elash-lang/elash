#pragma once

#include <elash/sema/symbol.h>
#include <elash/sema/scope.h>

#include <elash/hir/tree/module.h>
#include <elash/hir/tree/toplevel.h>
#include <elash/hir/tree/stmt.h>
#include <elash/hir/tree/expr.h>

#include <elash/mir/module.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>
#include <elash/mir/block.h>
#include <elash/mir/func.h>

#include <elash/util/dynarena.h>
#include <elash/sema/builtin.h>
#include <elash/diag/engine.h>

typedef struct ElLowerer {
    ElDynArena*   arena;
    ElDiagEngine* diag;
    ElBuiltins*   builtins;

    uint32_t current_block_id;
    ElMirFunc* current_func;
    ElMirModule* current_mod;
    ElMirInstrBuf ibuf;

    ElMirValue** symbol_map;

    uint32_t break_target_id;
    uint32_t continue_target_id;
} ElLowerer;

void el_lowerer_init(ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag, ElBuiltins* builtins);
void el_lowerer_free(ElLowerer* lw);

bool el_lowerer_has_terminator(ElLowerer* lw);
void el_lowerer_emit_block(ElLowerer* lw, uint32_t id);

ElMirValue*  el_lowerer_lower_expr(ElLowerer* lw, ElHirExprNode* hir);
void         el_lowerer_lower_stmt(ElLowerer* lw, ElHirStmtNode* hir);
void         el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir);
ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir);

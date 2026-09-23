#pragma once

#include <elash/lowerer/builtin.h>

#include <elash/hir/symbol.h>
#include <elash/hir/scope.h>

#include <elash/hir/tree/module.h>
#include <elash/hir/tree/decl.h>
#include <elash/hir/tree/stmt.h>
#include <elash/hir/tree/expr.h>

#include <elash/mir/module.h>
#include <elash/mir/instr.h>
#include <elash/mir/value.h>
#include <elash/mir/block.h>
#include <elash/mir/func.h>
#include <elash/mir/symbol.h>
#include <elash/mir/type.h>

#include <elash/sema/backends.h>
#include <elash/prof/prof.h>

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>
#include <elash/sema/tcache.h>

typedef struct ElLowerer {
    ElDynArena*   arena;
    ElDiagEngine* diag;
    ElTypeCache*  tcache;
    ElBSQuery*    bsquery;

    ElLowererBuiltins* builtins;

    ElProfState* prof;
    ElProfSubstage
        *pss_expr,
        *pss_stmt,
        *pss_decl,
        *pss_type;

    uint32_t current_block_id;
    ElMirFunc* current_func;
    ElMirModule* current_mod;
    ElMirInstrBuf ibuf;

    ElMirValue** symbol_map;
    ElMirSymbol** mir_symbol_map;
    uint32_t next_sym_id; // starts at hir->sym_count; used for anonymous symbols

    uint32_t break_target_id;
    uint32_t continue_target_id;
} ElLowerer;

void el_lowerer_init(
    ElLowerer* lw, ElDynArena* arena, ElDiagEngine* diag,
    ElTypeCache* tcache, ElBSQuery* bsquery, ElLowererBuiltins* builtins,
    ElProfState* prof
);
void el_lowerer_free(ElLowerer* lw);

bool el_lowerer_has_terminator(ElLowerer* lw);
void el_lowerer_emit_block(ElLowerer* lw, uint32_t id);

ElMirValue*  el_lowerer_get_lvalue(ElLowerer* lw, ElHirExpr* hir);

void         el_lower_global_decl(ElLowerer* lw, ElHirDecl* decl);
void         el_lower_local_decl(ElLowerer* lw, ElHirDecl* decl);
ElMirValue*  el_lower_expr(ElLowerer* lw, ElHirExpr* hir);
void         el_lower_stmt(ElLowerer* lw, ElHirStmt* hir);
ElMirModule* el_lower_module(ElLowerer* lw, ElHirModule* hir);

ElMirType* el_lowerer_map_type_raw(ElTypeCache* tcache, const ElHirType* type);

ElMirType*   el_lowerer_map_type(ElLowerer* lw, const ElHirType* type);
ElMirSymbol* el_lowerer_map_symbol(ElLowerer* lw, ElHirSymbol* sym);

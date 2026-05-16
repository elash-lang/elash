#pragma once

#include <elash/sema/builtin.h>
#include <elash/sema/symbol.h>
#include <elash/sema/scope.h>

#include <elash/hir/tree/module.h>
#include <elash/hir/tree/toplevel.h>
#include <elash/hir/tree/stmt.h>
#include <elash/hir/tree/expr.h>

#include <elash/ast/tree/module.h>
#include <elash/ast/tree/toplevel.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

typedef struct ElBinderInitOpts {
    ElBuiltins* builtins;
    ElDiagEngine* diag;
    ElDynArena* hir_arena;
    ElDynArena* sym_arena;
    ElDynArena* type_arena;
} ElBinderInitOpts;

typedef struct ElBinder {
    ElDynArena* hir_arena;
    ElDynArena* sym_arena;
    ElDynArena* type_arena;

    ElDiagEngine* diag;
    ElBuiltins* builtins;

    ElScope* builtin_scope;
    ElScope* global_scope;

    ElScope* current_scope;
    ElSymbol* current_func;

    uint32_t sym_id_counter;
    uint32_t loop_depth;
} ElBinder;

#define el_binder_init(BINDER, ...) \
    el_binder_init_opts((BINDER), (ElBinderInitOpts) { __VA_ARGS__ })

void el_binder_init_opts(ElBinder* binder, ElBinderInitOpts opts);
void el_binder_free(ElBinder* binder);

ElScope* _el_binder_push_scope(ElBinder* binder);
ElScope* _el_binder_pop_scope(ElBinder* binder);

ElHirBlockStmtNode _el_binder_bind_block(ElBinder* binder, ElAstBlockStmtNode* in);
ElType*            _el_binder_bind_type(ElBinder* binder, ElAstTypeNode* node);

ElHirExprNode*     el_binder_bind_expr(ElBinder* binder,   ElAstExprNode* in);
ElHirStmtNode*     el_binder_bind_stmt(ElBinder* binder,   ElAstStmtNode* in);
ElHirTopLevelNode* el_binder_bind_toplvl(ElBinder* binder, ElAstTopLevelNode* in);
ElHirModule*       el_binder_bind_module(ElBinder* binder, ElAstModuleNode* in);

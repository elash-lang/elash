#pragma once

#include <elash/binder/builtin.h>

#include <elash/hir/symbol.h>
#include <elash/hir/scope.h>
#include <elash/hir/toe.h>

#include <elash/hir/tree/module.h>
#include <elash/hir/tree/decl.h>
#include <elash/hir/tree/stmt.h>
#include <elash/hir/tree/expr.h>

#include <elash/ast/tree/module.h>
#include <elash/ast/tree/decl.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

typedef struct ElBinderInitOpts {
    ElBinderBuiltins* builtins;
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
    ElBinderBuiltins* builtins;

    ElScope* builtin_scope;
    ElScope* global_scope;

    ElScope* current_scope;
    ElHirSymbol* current_func;

    uint32_t sym_id_counter;
    uint32_t loop_depth;
} ElBinder;

#define el_binder_init(BINDER, ...) \
    el_binder_init_opts((BINDER), (ElBinderInitOpts) { __VA_ARGS__ })

void el_binder_init_opts(ElBinder* binder, ElBinderInitOpts opts);
void el_binder_free(ElBinder* binder);

ElScope* _el_binder_push_scope(ElBinder* binder);
ElScope* _el_binder_pop_scope(ElBinder* binder);

ElHirBlockStmt _el_binder_bind_block(ElBinder* binder, ElAstBlockStmt* in);
ElHirType*     _el_binder_bind_type(ElBinder* binder, ElAstType* type);

ElHirExpr* _el_binder_simplify_expr(ElBinder* binder, ElHirExpr* expr);
ElHirExpr* _el_binder_explicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);
ElHirExpr* _el_binder_implicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);
ElHirExpr* _el_binder_apply_default_type(ElBinder* binder, ElHirExpr* expr);


ElHirExpr* el_binder_bind_builtin_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call, ElHirSymbol* builtin);
ElHirExpr* el_binder_bind_init(ElBinder* binder, ElAstInit* in, ElHirType* expected_type);

ElHirToE*      el_binder_bind_toe(ElBinder* binder,    ElAstToE* in);
ElHirExpr*     el_binder_bind_expr(ElBinder* binder,   ElAstExpr* in);
ElHirDecl*     el_binder_bind_decl(ElBinder* binder,   ElAstDecl* in);
ElHirStmt*     el_binder_bind_stmt(ElBinder* binder,   ElAstStmt* in);
ElHirModule*   el_binder_bind_module(ElBinder* binder, ElAstModule* in);

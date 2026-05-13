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

typedef struct ElBinder {
    ElDynArena* arena;
    ElDiagEngine* diag;
    
    ElBuiltins* builtins;

    ElScope* builtin_scope;
    ElScope* global_scope;

    ElScope* current_scope;
    ElSymbol* current_func;

    uint32_t sym_id_counter;
} ElBinder;

void el_binder_init(ElBinder* binder, ElDynArena* arena, ElDiagEngine* diag, ElBuiltins* builtins);
void el_binder_free(ElBinder* binder);

ElScope* _el_binder_push_scope(ElBinder* binder);
ElScope* _el_binder_pop_scope(ElBinder* binder);

ElHirBlockStmtNode _el_binder_bind_block(ElBinder* binder, ElAstBlockStmtNode* in);
ElType*            _el_binder_bind_type(ElBinder* binder, ElAstTypeNode* node);

ElHirExprNode*     el_binder_bind_expr(ElBinder* binder,   ElAstExprNode* in);
ElHirStmtNode*     el_binder_bind_stmt(ElBinder* binder,   ElAstStmtNode* in);
ElHirTopLevelNode* el_binder_bind_toplvl(ElBinder* binder, ElAstTopLevelNode* in);
ElHirModule*       el_binder_bind_module(ElBinder* binder, ElAstModuleNode* in);

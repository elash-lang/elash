#pragma once

#include <elash/sema/symbol.h>
#include <elash/sema/scope.h>

#include <elash/hir/tree/module.h>
#include <elash/hir/tree/toplevel.h>
#include <elash/hir/tree/stmt.h>
#include <elash/hir/tree/expr.h>

#include <elash/ast/module.h>
#include <elash/ast/toplevel.h>
#include <elash/ast/stmt.h>
#include <elash/ast/expr.h>

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

typedef struct ElBinder {
    ElDynArena* arena;
    ElDiagEngine* diag;
    ElScope* builtin_scope;
    ElScope* global_scope;
    ElScope* current_scope;

    uint32_t sym_id_counter;

    ElType* type_int;
    ElType* type_uint;
    ElType* type_char;
    ElType* type_bool;
} ElBinder;

void el_binder_init(ElBinder* binder, ElDynArena* arena, ElDiagEngine* diag);
void el_binder_free(ElBinder* binder);

ElScope* _el_binder_push_scope(ElBinder* binder);
ElScope* _el_binder_pop_scope(ElBinder* binder);

ElHirExprNode*     el_binder_bind_expr(ElBinder* binder,   ElAstExprNode* in);
ElHirStmtNode*     el_binder_bind_stmt(ElBinder* binder,   ElAstStmtNode* in);
ElHirTopLevelNode* el_binder_bind_toplvl(ElBinder* binder, ElAstTopLevelNode* in);
ElHirModule*       el_binder_bind_module(ElBinder* binder, ElAstModuleNode* in);

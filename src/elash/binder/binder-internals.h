#pragma once
#include <elash/binder/binder.h> // IWYU pragma: export
#include <elash/util/assert.h>   // IWYU pragma: export
#include <elash/diag/engine.h>   // IWYU pragma: export
#include <elash/util/int128.h>   // IWYU pragma: export

ElHirBlockStmt _el_bind_block(ElBinder* binder, ElAstBlockStmt* in);
ElHirToE* _el_bind_unresolved(ElBinder* binder, ElAstToE* in, ElAstUnr* unr);

//////////// constant folding and casts //////////////
ElInt128   _el_binder_wrap_typed_int(ElBinder* binder, ElSourceSpan span, ElHirType* type, ElInt128 value);
ElHirExpr* _el_binder_explicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);
ElHirExpr* _el_binder_implicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);
ElHirExpr* _el_binder_eval_const_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);
ElHirExpr* _el_binder_apply_default_type(ElBinder* binder, ElHirExpr* expr);
ElHirExpr* _el_binder_simplify_expr(ElBinder* binder, ElHirExpr* expr);

///////////////// initializers ///////////////////
ElHirExpr* _el_bind_designated(ElBinder* binder, ElAstInit* in, ElHirType* expected_type, ElStorageClass scls);
ElHirExpr* _el_bind_init_list(ElBinder* binder, ElAstInit* in, ElHirType* expected_type, ElStorageClass scls);

/////////////////// helpers //////////////////////
usize _el_binder_find_field(ElStringView name, const ElHirStructType* type, bool* found);
bool _el_binder_eval_const_index(ElBinder* binder, ElAstExpr* expr, usize* out_idx);
bool _el_binder_is_const(ElBinder* binder, ElHirExpr* expr);

bool _el_binder_ensure_params(ElBinder* binder, ElAstExpr* in, usize count, ElStringView bname);
ElHirToE* _el_binder_ensure_toe(ElBinder* binder, ElAstToI* toi, ElStringView bname);

usize _el_binder_sizeof(ElBinder* binder, ElHirType* type);
usize _el_binder_alignof(ElBinder* binder, ElHirType* type);

ElScope* _el_binder_push_scope(ElBinder* binder);
ElScope* _el_binder_pop_scope(ElBinder* binder);

///////////////// analysis /////////////////////
typedef enum Redundancy {
    REDUNDANCY_NONE,    //< expressions with side effects;            i.e. foo();
    REDUNDANCY_PARTIAL, //< has side effects but partially redundant; i.e. 1 + foo();
    REDUNDANCY_FULL,    //< zero side effects, can be safely removed; i.e. 2 * 2;
} Redundancy;

bool _el_binder_stmt_always_returns(ElBinder* binder, ElHirStmt* stmt);
bool _el_binder_block_always_returns(ElBinder* binder, ElHirBlockStmt block);

Redundancy _el_binder_redundancy_if_ignored(ElBinder* binder, ElHirExpr* expr);
bool _el_binder_ensure_complete(ElBinder* binder, ElSourceSpan span, ElHirType* type);

////////////// builtins //////////////
typedef ElHirExpr* BuiltinHandler(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call);

BuiltinHandler _el_bind_len_call;
BuiltinHandler _el_bind_mkslice_call;

BuiltinHandler _el_bind_sizeof;
BuiltinHandler _el_bind_alignof;

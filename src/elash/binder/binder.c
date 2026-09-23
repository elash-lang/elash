#include "binder-internals.h"

#include <elash/util/assert.h>

#include <elash/sema/backends.h>
#include <elash/sema/tcache.h>

static void register_builtin_type(ElBinder* binder, ElStringView name, ElHirType* type) {
    ElHirSymbol* sym = el_hir_new_type_symbol(binder->arena, binder->sym_id_counter++, name, type);
    (void) el_hir_scope_insert(binder->builtin_scope, sym);
}

static void register_builtin_func(ElBinder* binder, ElStringView name, ElBuiltinKind kind) {
    ElHirSymbol* sym = el_hir_new_builtin_symbol(binder->arena, binder->sym_id_counter++, name, kind);
    (void) el_hir_scope_insert(binder->builtin_scope, sym);
}

void el_binder_init_opts(ElBinder* binder, ElBinderInitOpts opts) {
    EL_ASSERT(opts.diag != NULL, "should not be null");
    EL_ASSERT(opts.arena != NULL, "should not be null");
    EL_ASSERT(opts.tcache != NULL, "should not be null");
    EL_ASSERT(opts.bsquery != NULL, "should not be null");

    binder->builtins = opts.builtins;
    binder->arena    = opts.arena;
    binder->diag     = opts.diag;
    binder->prof    = opts.prof;

    binder->tcache  = opts.tcache;
    binder->bsquery = opts.bsquery;

    binder->loop_depth = 0;
    binder->sym_id_counter = 0;
    binder->current_func = NULL;

    if (binder->prof != NULL) {
        binder->pss_expr = el_prof_new_sub(binder->prof, EL_SV("Binding expressions"));
        binder->pss_stmt = el_prof_new_sub(binder->prof, EL_SV("Binding statements"));
        binder->pss_decl = el_prof_new_sub(binder->prof, EL_SV("Binding declarations"));
        binder->pss_type = el_prof_new_sub(binder->prof, EL_SV("Binding types"));
        binder->pss_init = el_prof_new_sub(binder->prof, EL_SV("Binding initializers"));
        binder->pss_toi  = el_prof_new_sub(binder->prof, EL_SV("Binding ToIs"));
        binder->pss_toe  = el_prof_new_sub(binder->prof, EL_SV("Binding ToEs"));
    }

    binder->builtin_scope = el_hir_scope_new(NULL);
    register_builtin_type(binder, EL_SV("void"), binder->builtins->type_void);
    register_builtin_type(binder, EL_SV("char"), binder->builtins->type_char);
    register_builtin_type(binder, EL_SV("bool"), binder->builtins->type_bool);

    register_builtin_type(binder, EL_SV("isize"),   binder->builtins->type_isize);
    register_builtin_type(binder, EL_SV("usize"),   binder->builtins->type_usize);
    register_builtin_type(binder, EL_SV("int"),     binder->builtins->type_int);
    register_builtin_type(binder, EL_SV("uint"),    binder->builtins->type_uint);
    register_builtin_type(binder, EL_SV("int8"),    binder->builtins->type_int8);
    register_builtin_type(binder, EL_SV("uint8"),   binder->builtins->type_uint8);
    register_builtin_type(binder, EL_SV("int16"),   binder->builtins->type_int16);
    register_builtin_type(binder, EL_SV("uint16"),  binder->builtins->type_uint16);
    register_builtin_type(binder, EL_SV("int32"),   binder->builtins->type_int32);
    register_builtin_type(binder, EL_SV("uint32"),  binder->builtins->type_uint32);
    register_builtin_type(binder, EL_SV("int64"),   binder->builtins->type_int64);
    register_builtin_type(binder, EL_SV("uint64"),  binder->builtins->type_uint64);
    register_builtin_type(binder, EL_SV("int128"),  binder->builtins->type_int128);
    register_builtin_type(binder, EL_SV("uint128"), binder->builtins->type_uint128);

    register_builtin_type(binder, EL_SV("float"),    binder->builtins->type_float);
    register_builtin_type(binder, EL_SV("float16"),  binder->builtins->type_float16);
    register_builtin_type(binder, EL_SV("float32"),  binder->builtins->type_float32);
    register_builtin_type(binder, EL_SV("float64"),  binder->builtins->type_float64);
    register_builtin_type(binder, EL_SV("float128"), binder->builtins->type_float128);

    register_builtin_func(binder, EL_SV("len"), EL_BUILTIN_LEN);
    register_builtin_func(binder, EL_SV("mkslice"), EL_BUILTIN_MKSLICE);

    register_builtin_func(binder, EL_SV("sizeof"), EL_BUILTIN_SIZEOF);
    register_builtin_func(binder, EL_SV("alignof"), EL_BUILTIN_ALIGNOF);

    binder->global_scope = el_hir_scope_new(binder->builtin_scope);
    binder->current_scope = binder->global_scope;
}

void el_binder_free(ElBinder* binder) {
    el_hir_scope_free(binder->global_scope);
    el_hir_scope_free(binder->builtin_scope);
}

ElScope* _el_binder_push_scope(ElBinder* binder) {
    ElScope* scope = el_hir_scope_new(binder->current_scope);
    binder->current_scope = scope;
    return scope;
}

ElScope* _el_binder_pop_scope(ElBinder* binder) {
    ElScope* parent = binder->current_scope->parent;
    el_hir_scope_free(binder->current_scope);
    return binder->current_scope = parent;
}

usize _el_binder_find_field(ElStringView name, const ElHirStructType* type, bool* found) {
    *found = false;
    usize idx = 0;
    for (usize i = 0; i < type->count; i++) {
        if (el_sv_eql(type->fields[i].name, name)) {
            idx = i;
            *found = true;
            break;
        }
    }
    return idx;
}

bool _el_binder_eval_const_index(ElBinder* binder, ElAstExpr* expr, usize* out_idx) {
    ElHirExpr* bound = el_bind_expr(binder, expr);
    if (bound == NULL) return false;

    ElInt128 val;
    if (bound->kind == EL_HIR_EXPR_LITERAL && bound->as.literal.kind == EL_HIR_LITERAL_INT) {
        val = bound->as.literal.of.int_;
        goto common;
    }

    if (bound->kind == EL_HIR_EXPR_CONST) {
        val = bound->as.constant.as.int_;
        goto common;
    }

    return false;

common:
    // TODO: this sucks. we should add some helper like el_i128_sign or el_128_is_neg
    if (el_i128_lt(val, EL_INT128(0))) return false;

    if (el_i128_ge(val, EL_INT128(SIZE_MAX))) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.overflow",
            bound->span, "integer constant value too high to represent an array index"
        );
    }

    *out_idx = (usize)el_i128_lo(val);
    return true;

}

usize _el_binder_sizeof(ElBinder* binder, ElHirType* type) {
    ElBSType* bstype = el_tcache_get_bst_from_hir(binder->tcache, type);
    return binder->bsquery->get_size(binder->bsquery, bstype);
}

usize _el_binder_alignof(ElBinder* binder, ElHirType* type) {
    ElBSType* bstype = el_tcache_get_bst_from_hir(binder->tcache, type);
    return binder->bsquery->get_align(binder->bsquery, bstype);
}

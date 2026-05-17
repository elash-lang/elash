#include <elash/binder/binder.h>
#include <elash/util/assert.h>

static void register_builtin_type(ElBinder* binder, ElStringView name, ElType* type) {
    ElSymbol* sym = el_sema_new_type_symbol(binder->sym_arena, binder->sym_id_counter++, name, type);
    (void) el_sema_scope_insert(binder->builtin_scope, sym);
}

static void register_builtin_func(ElBinder* binder, ElStringView name, ElBuiltinKind kind) {
    ElSymbol* sym = el_sema_new_builtin_symbol(binder->sym_arena, binder->sym_id_counter++, name, kind);
    (void) el_sema_scope_insert(binder->builtin_scope, sym);
}

void el_binder_init_opts(ElBinder* binder, ElBinderInitOpts opts) {
    binder->hir_arena  = opts.hir_arena;
    binder->sym_arena  = opts.sym_arena;
    binder->type_arena = opts.type_arena;
    binder->builtins   = opts.builtins;
    binder->diag       = opts.diag;

    binder->loop_depth = 0;
    binder->sym_id_counter = 0;
    binder->current_func = NULL;

    binder->builtin_scope = el_sema_scope_new(NULL);
    register_builtin_type(binder, EL_SV("void"), binder->builtins->type_void);
    register_builtin_type(binder, EL_SV("int"),  binder->builtins->type_int);
    register_builtin_type(binder, EL_SV("uint"), binder->builtins->type_uint);
    register_builtin_type(binder, EL_SV("char"), binder->builtins->type_char);
    register_builtin_type(binder, EL_SV("bool"), binder->builtins->type_bool);

    register_builtin_func(binder, EL_SV("len"), EL_BUILTIN_LEN);

    binder->global_scope = el_sema_scope_new(binder->builtin_scope);
    binder->current_scope = binder->global_scope;
}

void el_binder_free(ElBinder* binder) {
    el_sema_scope_free(binder->global_scope);
    el_sema_scope_free(binder->builtin_scope);
}

ElScope* _el_binder_push_scope(ElBinder* binder) {
    ElScope* scope = el_sema_scope_new(binder->current_scope);
    binder->current_scope = scope;
    return scope;
}

ElScope* _el_binder_pop_scope(ElBinder* binder) {
    ElScope* parent = binder->current_scope->parent;
    el_sema_scope_free(binder->current_scope);
    return binder->current_scope = parent;
}

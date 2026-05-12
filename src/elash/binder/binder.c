#include <elash/binder/binder.h>
#include <elash/util/assert.h>

static ElType* _el_binder_register_builtin_type(ElBinder* binder, ElStringView name, ElPrimitiveTypeKind kind) {
    ElType* type = el_sema_new_prim_type(binder->arena, kind);
    ElSymbol* sym = el_sema_new_type_symbol(binder->arena, binder->sym_id_counter++, name, type);
    (void) el_sema_scope_insert(binder->builtin_scope, sym);
    return type;
}

void el_binder_init(ElBinder* binder, ElDynArena* arena, ElDiagEngine* diag) {
    binder->arena = arena;
    binder->diag = diag;
    binder->sym_id_counter = 0;

    binder->builtin_scope = el_sema_scope_new(NULL);

    binder->type_int  = _el_binder_register_builtin_type(binder, EL_SV("int"),  EL_PRIMTYPE_INT);
    binder->type_uint = _el_binder_register_builtin_type(binder, EL_SV("uint"), EL_PRIMTYPE_UINT);
    binder->type_char = _el_binder_register_builtin_type(binder, EL_SV("char"), EL_PRIMTYPE_CHAR);
    binder->type_bool = _el_binder_register_builtin_type(binder, EL_SV("bool"), EL_PRIMTYPE_BOOL);

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

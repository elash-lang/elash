#include <elash/sema/builtin.h>

void el_sema_init_builtins(ElBuiltins* builtins, ElDynArena* arena) {
    builtins->type_void = el_sema_new_prim_type(arena, EL_PRIMTYPE_VOID);
    builtins->type_int  = el_sema_new_prim_type(arena, EL_PRIMTYPE_INT);
    builtins->type_uint = el_sema_new_prim_type(arena, EL_PRIMTYPE_UINT);
    builtins->type_char = el_sema_new_prim_type(arena, EL_PRIMTYPE_CHAR);
    builtins->type_bool = el_sema_new_prim_type(arena, EL_PRIMTYPE_BOOL);
}

#include <elash/sema/builtin.h>

void el_sema_init_builtins(ElBuiltins* builtins, ElDynArena* arena) {
    builtins->type_void = el_sema_new_prim_type(arena, EL_PRIMTYPE_VOID);
    builtins->type_char = el_sema_new_prim_type(arena, EL_PRIMTYPE_CHAR);
    builtins->type_bool = el_sema_new_prim_type(arena, EL_PRIMTYPE_BOOL);

#define INIT_INT_PAIR(SNAME, UNAME, WIDTH) \
    builtins->type_##SNAME = el_sema_new_int_type(arena, WIDTH, /*is_signed=*/true); \
    builtins->type_##UNAME = el_sema_new_int_type(arena, WIDTH, /*is_signed=*/false)

    INIT_INT_PAIR(int,    uint,    EL_INT_WIDTH_EFFICIENT);
    INIT_INT_PAIR(isize,  usize,   EL_INT_WIDTH_NATIVE);
    INIT_INT_PAIR(int8,   uint8,   EL_INT_WIDTH_8);
    INIT_INT_PAIR(int16,  uint16,  EL_INT_WIDTH_16);
    INIT_INT_PAIR(int32,  uint32,  EL_INT_WIDTH_32);
    INIT_INT_PAIR(int64,  uint64,  EL_INT_WIDTH_64);
    INIT_INT_PAIR(int128, uint128, EL_INT_WIDTH_128);

#undef INIT_INT_PAIR
}

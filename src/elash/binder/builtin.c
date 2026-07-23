#include <elash/binder/builtin.h>

void el_binder_init_builtins(ElBinderBuiltins* builtins, ElDynArena* arena) {
    builtins->type_void = el_hir_new_prim_type(arena, EL_PRIMTYPE_VOID);
    builtins->type_char = el_hir_new_prim_type(arena, EL_PRIMTYPE_CHAR);
    builtins->type_bool = el_hir_new_prim_type(arena, EL_PRIMTYPE_BOOL);

#define INIT_INT_PAIR(SNAME, UNAME, WIDTH) \
    builtins->type_##SNAME = el_hir_new_int_type(arena, WIDTH, /*is_signed=*/true); \
    builtins->type_##UNAME = el_hir_new_int_type(arena, WIDTH, /*is_signed=*/false)

    INIT_INT_PAIR(int,    uint,    EL_HIR_IWIDTH_EFFICIENT);
    INIT_INT_PAIR(isize,  usize,   EL_HIR_IWIDTH_NATIVE);
    INIT_INT_PAIR(int8,   uint8,   EL_HIR_IWIDTH_8);
    INIT_INT_PAIR(int16,  uint16,  EL_HIR_IWIDTH_16);
    INIT_INT_PAIR(int32,  uint32,  EL_HIR_IWIDTH_32);
    INIT_INT_PAIR(int64,  uint64,  EL_HIR_IWIDTH_64);
    INIT_INT_PAIR(int128, uint128, EL_HIR_IWIDTH_128);

#undef INIT_INT_PAIR

    builtins->type_float    = el_hir_new_float_type(arena, EL_HIR_FPWIDTH_EFFICIENT);
    builtins->type_float16  = el_hir_new_float_type(arena, EL_HIR_FPWIDTH_16);
    builtins->type_float32  = el_hir_new_float_type(arena, EL_HIR_FPWIDTH_32);
    builtins->type_float64  = el_hir_new_float_type(arena, EL_HIR_FPWIDTH_64);
    builtins->type_float128 = el_hir_new_float_type(arena, EL_HIR_FPWIDTH_128);
}

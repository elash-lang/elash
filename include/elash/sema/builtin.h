#pragma once

#include <elash/util/dynarena.h>
#include <elash/sema/type.h>

typedef struct ElBuiltins {
    ElType *type_int,    *type_uint;
    ElType *type_isize,  *type_usize;
    ElType *type_int8,   *type_uint8;
    ElType *type_int16,  *type_uint16;
    ElType *type_int32,  *type_uint32;
    ElType *type_int64,  *type_uint64;
    ElType *type_int128, *type_uint128;

    ElType* type_char;
    ElType* type_bool;
    ElType* type_void;
} ElBuiltins;

void el_sema_init_builtins(ElBuiltins* builtins, ElDynArena* arena);

#pragma once

#include <elash/util/dynarena.h>
#include <elash/hir/type.h>

typedef struct ElBinderBuiltins {
    ElHirType *type_int,    *type_uint;
    ElHirType *type_isize,  *type_usize;
    ElHirType *type_int8,   *type_uint8;
    ElHirType *type_int16,  *type_uint16;
    ElHirType *type_int32,  *type_uint32;
    ElHirType *type_int64,  *type_uint64;
    ElHirType *type_int128, *type_uint128;

    ElHirType* type_char;
    ElHirType* type_bool;
    ElHirType* type_void;
} ElBinderBuiltins;

void el_binder_init_builtins(ElBinderBuiltins* builtins, ElDynArena* arena);

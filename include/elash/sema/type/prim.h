#pragma once

#include <elash/util/dynarena.h>

typedef struct ElType ElType;

typedef enum ElBasicIntWidth {
    EL_INT_WIDTH_NATIVE,    // isize/usize
    EL_INT_WIDTH_EFFICIENT, // int/uint
    EL_INT_WIDTH_8,         // int8/uint8
    EL_INT_WIDTH_16,        // int16/uint16
    EL_INT_WIDTH_32,        // int32/uint32
    EL_INT_WIDTH_64,        // int64/uint64
    EL_INT_WIDTH_128,       // int128/uint128
} ElBasicIntWidth;

typedef enum ElPrimitiveTypeKind {
    EL_PRIMTYPE_VOID,
    EL_PRIMTYPE_INT,
    EL_PRIMTYPE_CHAR,
    EL_PRIMTYPE_BOOL,
} ElPrimitiveTypeKind;

typedef struct ElPrimitiveType {
    ElPrimitiveTypeKind kind;
    union {
        struct { // for EL_PRIMTYPE_INT and EL_PRIMTYPE_UINT
            ElBasicIntWidth width;
            bool is_signed;
        } integral;
    } as;
} ElPrimitiveType;

ElType* el_sema_new_prim_type(ElDynArena* arena, ElPrimitiveTypeKind kind);
ElType* el_sema_new_int_type(ElDynArena* arena, ElBasicIntWidth width, bool is_signed);

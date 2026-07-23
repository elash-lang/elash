#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirType ElHirType;

typedef enum ElHirIntWidth {
    EL_HIR_IWIDTH_NATIVE,    // isize/usize
    EL_HIR_IWIDTH_EFFICIENT, // int/uint
    EL_HIR_IWIDTH_8,         // int8/uint8
    EL_HIR_IWIDTH_16,        // int16/uint16
    EL_HIR_IWIDTH_32,        // int32/uint32
    EL_HIR_IWIDTH_64,        // int64/uint64
    EL_HIR_IWIDTH_128,       // int128/uint128
} ElHirIntWidth;

typedef enum ElHirFpWidth {
    EL_HIR_FPWIDTH_EFFICIENT, // float
    EL_HIR_FPWIDTH_16,        // float16
    EL_HIR_FPWIDTH_32,        // float32
    EL_HIR_FPWIDTH_64,        // float64
    EL_HIR_FPWIDTH_128,       // float128
} ElHirFpWidth;

typedef enum ElHirPrimTypeKind {
    EL_PRIMTYPE_VOID,
    EL_PRIMTYPE_INT,
    EL_PRIMTYPE_CHAR,
    EL_PRIMTYPE_BOOL,
    EL_PRIMTYPE_FLOAT,
} ElHirPrimTypeKind;

typedef struct ElHirPrimType {
    ElHirPrimTypeKind kind;
    union {
        struct { // for EL_PRIMTYPE_INT
            ElHirIntWidth width;
            bool is_signed;
        } integral;
        struct {
            ElHirFpWidth width;
        } fp;
    } as;
} ElHirPrimType;

ElHirType* el_hir_new_prim_type(ElDynArena* arena, ElHirPrimTypeKind kind);
ElHirType* el_hir_new_int_type(ElDynArena* arena, ElHirIntWidth width, bool is_signed);
ElHirType* el_hir_new_float_type(ElDynArena* arena, ElHirFpWidth width);

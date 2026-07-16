#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef enum ElHirIntrKind {
    EL_HIR_INTR_SLICE_LEN,
    EL_HIR_INTR_SLICE_DATA,
    EL_HIR_INTR_MAKE_SLICE,
} ElHirIntrKind;

typedef struct ElHirIntrExpr {
    ElHirIntrKind kind;
    union {
        ElHirExpr* slice;
        struct {
            ElHirExpr* rwslice;
            ElHirExpr* len;
        };
    } params;
} ElHirIntrExpr;

ElHirExpr* el_hir_new_slice_len_intr(ElDynArena* arena, ElHirType* usize_type, ElHirExpr* slice);
ElHirExpr* el_hir_new_slice_data_intr(ElDynArena* arena, ElHirType* usize_type, ElHirExpr* slice);
ElHirExpr* el_hir_new_make_slice_intr(ElDynArena* arena, ElHirExpr* rwslice, ElHirExpr* len);

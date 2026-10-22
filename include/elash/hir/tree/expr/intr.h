#pragma once

#include <elash/util/dynarena.h>
#include <elash/source/span.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef enum ElHirIntrKind {
    EL_HIR_INTR_SLICE_LEN,
    EL_HIR_INTR_SLICE_DATA,
    EL_HIR_INTR_MAKE_SLICE,
    EL_HIR_INTR_EXCHANGE,

    EL_HIR_INTR_NULL_OPT,
    EL_HIR_INTR_SOME_OPT,
} ElHirIntrKind;

typedef struct ElHirIntrExpr {
    ElHirIntrKind kind;
    union {
        ElHirExpr* value;
        ElHirExpr* slice;
        struct {
            ElHirExpr* rwslice;
            ElHirExpr* len;
        };
        struct {
            ElHirExpr* dest;
            ElHirExpr* value;
        } exchange;
    } params;
} ElHirIntrExpr;

ElHirExpr* el_hir_new_slice_len_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* usize_type, ElHirExpr* slice);
ElHirExpr* el_hir_new_slice_data_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* rwslice_type, ElHirExpr* slice);
ElHirExpr* el_hir_new_make_slice_intr(ElDynArena* arena, ElSourceSpan span, ElHirExpr* rwslice, ElHirExpr* len);
ElHirExpr* el_hir_new_exchange_intr(ElDynArena* arena, ElSourceSpan span, ElHirExpr* dest, ElHirExpr* value);

ElHirExpr* el_hir_new_null_opt_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* opt_type);
ElHirExpr* el_hir_new_some_opt_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* opt_type, ElHirExpr* value);

#include <elash/hir/tree/expr.h>
#include <elash/hir/type.h>

#include <elash/util/assert.h>

#define NICE_TO_HAVE_ASSERTIONS(usize_type, slice) \
    EL_ASSERT(slice->type->kind == EL_HIR_TYPE_SLICE, "slice len intrinsic argument must be a slice"); \
    EL_ASSERT( \
        usize_type->kind == EL_HIR_TYPE_PRIM \
        && usize_type->as.prim.kind == EL_PRIMTYPE_INT \
        && usize_type->as.prim.as.integral.width == EL_HIR_IWIDTH_NATIVE, \
        "usize_type param must be instance of usize type" \
    );

ElHirExpr* el_hir_new_slice_len_intr(ElDynArena* arena, ElHirType* usize_type, ElHirExpr* slice) {
    NICE_TO_HAVE_ASSERTIONS(usize_type, slice);
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = usize_type,
        .as.intr = {
            .kind = EL_HIR_INTR_SLICE_LEN,
            .params.slice = slice,
        },
    });
}

ElHirExpr* el_hir_new_slice_data_intr(ElDynArena* arena, ElHirType* usize_type, ElHirExpr* slice) {
    NICE_TO_HAVE_ASSERTIONS(usize_type, slice);
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = usize_type,
        .as.intr = {
            .kind = EL_HIR_INTR_SLICE_DATA,
            .params.slice = slice,
        },
    });
}

ElHirExpr* el_hir_new_make_slice_intr(ElDynArena* arena, ElHirExpr* rwslice, ElHirExpr* len) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = el_hir_new_slice_type(arena, rwslice->type->as.rwslice.base),
        .as.intr = {
            .kind = EL_HIR_INTR_MAKE_SLICE,
            .params = { .rwslice = rwslice, .len = len },
        }
    });
}

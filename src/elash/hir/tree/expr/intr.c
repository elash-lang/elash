#include <elash/hir/tree/expr.h>
#include <elash/hir/type.h>

#include <elash/util/assert.h>

#define NICE_TO_HAVE_ASSERTIONS(SLICE_TYPE) \
    EL_ASSERT(el_hir_type_unwrap((SLICE_TYPE)->type)->kind == EL_HIR_TYPE_SLICE, "slice len intrinsic argument must be a slice"); \

#define ASSERT_USIZE(USIZE_TYPE)                                            \
    EL_ASSERT(                                                              \
        (USIZE_TYPE)->kind == EL_HIR_TYPE_PRIM                              \
        && (USIZE_TYPE)->as.prim.kind == EL_HIR_PRIMTYPE_INT                    \
        && (USIZE_TYPE)->as.prim.as.integral.width == EL_HIR_IWIDTH_NATIVE, \
        "(USIZE_TYPE) param must be instance of usize type"                 \
    );

ElHirExpr* el_hir_new_slice_len_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* usize_type, ElHirExpr* slice) {
    NICE_TO_HAVE_ASSERTIONS(slice);
    ASSERT_USIZE(usize_type);
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = usize_type,
        .span = span,
        .as.intr = {
            .kind = EL_HIR_INTR_SLICE_LEN,
            .params.slice = slice,
        },
    });
}

ElHirExpr* el_hir_new_slice_data_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* rwslice_type, ElHirExpr* slice) {
    NICE_TO_HAVE_ASSERTIONS(slice);
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = rwslice_type,
        .span = span,
        .as.intr = {
            .kind = EL_HIR_INTR_SLICE_DATA,
            .params.slice = slice,
        },
    });
}

ElHirExpr* el_hir_new_make_slice_intr(ElDynArena* arena, ElSourceSpan span, ElHirExpr* rwslice, ElHirExpr* len) {
    ElHirType* rwty = el_hir_type_canonical(rwslice->type);
    EL_ASSERT(rwty != NULL && rwty->kind == EL_HIR_TYPE_RWSLICE, "make-slice requires a raw slice");
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = el_hir_new_slice_type(arena, rwty->as.rwslice.base),
        .span = span,
        .as.intr = {
            .kind = EL_HIR_INTR_MAKE_SLICE,
            .params = { .rwslice = rwslice, .len = len },
        }
    });
}

ElHirExpr* el_hir_new_null_opt_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* opt_type) {
     return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = opt_type,
        .span = span,
        .as.intr = {
            .kind = EL_HIR_INTR_NULL_OPT,
        },
    });
}

ElHirExpr* el_hir_new_some_opt_intr(ElDynArena* arena, ElSourceSpan span, ElHirType* opt_type, ElHirExpr* value) {
     return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_INTR,
        .type = opt_type,
        .span = span,
        .as.intr = {
            .kind = EL_HIR_INTR_SOME_OPT,
            .params.value = value,
        },
    });
}

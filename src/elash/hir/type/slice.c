#include <elash/hir/type/slice.h>
#include <elash/hir/type.h>

ElHirType* el_hir_new_slice_type(ElDynArena* arena, ElHirType* base) {
    ElHirType* type = EL_DYNARENA_NEW(arena, ElHirType);
    type->kind = EL_HIR_TYPE_SLICE;
    type->as.slice.base = base;
    return type;
}

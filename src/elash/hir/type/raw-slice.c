#include <elash/hir/type/raw-slice.h>
#include <elash/hir/type.h>

ElHirType* el_hir_new_raw_slice_type(ElDynArena* arena, ElHirType* base) {
    ElHirType* type = EL_DYNARENA_NEW(arena, ElHirType);
    type->kind = EL_HIR_TYPE_RWSLICE;
    type->as.rwslice.base = base;
    return type;
}

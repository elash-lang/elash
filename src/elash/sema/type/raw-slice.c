#include <elash/sema/type/raw-slice.h>
#include <elash/sema/type.h>

ElType* el_sema_new_raw_slice_type(ElDynArena* arena, ElType* base) {
    ElType* type = EL_DYNARENA_NEW(arena, ElType);
    type->kind = EL_TYPE_RAW_SLICE;
    type->as.raw_slice.base = base;
    return type;
}


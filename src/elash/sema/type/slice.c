#include <elash/sema/type/slice.h>
#include <elash/sema/type.h>

ElType* el_sema_new_slice_type(ElDynArena* arena, ElType* base) {
    ElType* type = EL_DYNARENA_NEW(arena, ElType);
    type->kind = EL_TYPE_SLICE;
    type->as.slice.base = base;
    return type;
}

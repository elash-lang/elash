#include <elash/sema/type/array.h>
#include <elash/sema/type.h>

ElType* el_sema_new_array_type(ElDynArena* arena, ElType* base, usize size) {
    ElType* type = EL_DYNARENA_NEW(arena, ElType);
    type->kind = EL_TYPE_ARRAY;
    type->as.array.base = base;
    type->as.array.size = size;
    return type;
}

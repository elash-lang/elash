#include <elash/hir/type/array.h>
#include <elash/hir/type.h>

ElHirType* el_hir_new_array_type(ElDynArena* arena, ElHirType* base, usize size) {
    ElHirType* type = EL_DYNARENA_NEW(arena, ElHirType);
    type->kind = EL_HIR_TYPE_ARRAY;
    type->as.array.base = base;
    type->as.array.size = size;
    return type;
}

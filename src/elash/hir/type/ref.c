#include <elash/hir/type/ref.h>
#include <elash/hir/type.h>

ElHirType* el_hir_new_ref_type(ElDynArena* arena, ElHirType* base) {
    ElHirType* type = EL_DYNARENA_NEW(arena, ElHirType);
    type->kind = EL_HIR_TYPE_REF;
    type->as.ref.base = base;
    return type;
}

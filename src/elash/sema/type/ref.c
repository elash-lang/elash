#include <elash/sema/type/ref.h>
#include <elash/sema/type.h>

ElType* el_sema_new_ref_type(ElDynArena* arena, ElType* base) {
    ElType* type = EL_DYNARENA_NEW(arena, ElType);
    type->kind = EL_TYPE_REF;
    type->as.ref.base = base;
    return type;
}

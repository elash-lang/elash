#include <elash/hir/type.h>
#include <elash/util/assert.h>

ElHirType* el_hir_new_qual_type(ElDynArena* arena, ElHirType* base, ElMutabilitySpec mut) {
    EL_ASSERT(base != NULL, "qual base must not be null");
    EL_ASSERT(mut != EL_MUTSPEC_DEFAULT, "use the canonical type for default mutability");
    EL_ASSERT(base->kind != EL_HIR_TYPE_QUAL, "qual base must be canonical");

    return EL_DYNARENA_NEW_STRUCT(arena, ElHirType, {
        .kind = EL_HIR_TYPE_QUAL,
        .as.qual = { base, mut },
    });
}

#include <elash/hir/type.h>

ElHirType* el_hir_new_struct_type(ElDynArena* arena, ElHirStructField* fields, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirType, {
        .kind = EL_HIR_TYPE_STRUCT,
        .as.struct_ = { fields, count },
    });
}

#include <elash/mir/type.h>

ElMirType* el_mir_new_func_type(ElDynArena* arena, ElMirType* ret_type, ElMirType** params, usize param_count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirType, {
        .kind = EL_MIR_TYPE_FUNC,
        .as.func = {
            .ret_type = ret_type,
            .params = params,
            .param_count = param_count,
        },
    });
}

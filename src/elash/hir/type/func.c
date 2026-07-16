#include <elash/hir/type/func.h>
#include <elash/hir/type.h>

ElHirType* el_hir_new_func_type(ElDynArena* arena, ElHirType* ret_type, ElHirType** params, usize param_count) {
    ElHirType* type = EL_DYNARENA_NEW(arena, ElHirType);
    type->kind = EL_HIR_TYPE_FUNC;
    type->as.func.ret_type = ret_type;
    type->as.func.params = params;
    type->as.func.param_count = param_count;
    return type;
}

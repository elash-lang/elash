#include <elash/hir/symbol/func.h>
#include <elash/hir/symbol.h>
#include <elash/hir/type/func.h>

ElHirSymbol* el_hir_new_func_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElHirType* ret_type, ElHirSymbol** params, usize param_count
) {
    ElHirType** param_types = EL_DYNARENA_NEW_ARR(arena, ElHirType*, param_count);
    for (usize i = 0; i < param_count; i++) {
        param_types[i] = params[i]->as.var.type;
    }
    ElHirType* type = el_hir_new_func_type(arena, ret_type, param_types, param_count);

    ElHirSymbol* sym = EL_DYNARENA_NEW(arena, ElHirSymbol);
    sym->name = name;
    sym->kind = EL_SYM_FUNC;
    sym->id = id;
    sym->as.func = (ElHirFuncSymbol) {
        .type = type,
        .params = params,
        .param_count = param_count,
        .is_defined = false,
    };
    return sym;
}

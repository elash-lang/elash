#include <elash/sema/symbol/func.h>
#include <elash/sema/symbol.h>
#include <elash/sema/type/func.h>

ElSymbol* el_sema_new_func_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElType* ret_type, ElSymbol** params, usize param_count
) {
    ElType** param_types = EL_DYNARENA_NEW_ARR(arena, ElType*, param_count);
    for (usize i = 0; i < param_count; i++) {
        param_types[i] = params[i]->as.var.type;
    }
    ElType* type = el_sema_new_func_type(arena, ret_type, param_types, param_count);

    ElSymbol* sym = EL_DYNARENA_NEW(arena, ElSymbol);
    sym->name = name;
    sym->kind = EL_SYM_FUNC;
    sym->id = id;
    sym->as.func = (ElFuncSymbol) {
        .type = type,
        .ret_type = ret_type,
        .params = params,
        .param_count = param_count,
        .is_defined = false,
    };
    return sym;
}

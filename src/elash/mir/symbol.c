#include <elash/mir/symbol.h>

ElMirSymbol* el_mir_new_var_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElMirType* type) {
    ElMirSymbol* sym = EL_DYNARENA_NEW(arena, ElMirSymbol);
    sym->name = name;
    sym->kind = EL_MIR_SYM_VAR;
    sym->id = id;
    sym->as.var = (ElMirVarSymbol) {
        .type = type,
    };
    return sym;
}

ElMirSymbol* el_mir_new_func_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElMirType* ret_type, ElMirSymbol** params, usize param_count
) {
    ElMirType** param_types = EL_DYNARENA_NEW_ARR(arena, ElMirType*, param_count);
    for (usize i = 0; i < param_count; i++) {
        param_types[i] = params[i]->as.var.type;
    }
    ElMirType* type = el_mir_new_func_type(arena, ret_type, param_types, param_count);

    ElMirSymbol* sym = EL_DYNARENA_NEW(arena, ElMirSymbol);
    sym->name = name;
    sym->kind = EL_MIR_SYM_FUNC;
    sym->id = id;
    sym->as.func = (ElMirFuncSymbol) {
        .type = type,
        .params = params,
        .param_count = param_count,
        .is_defined = false,
    };
    return sym;
}

void el_mir_dump_symbol(const ElMirSymbol* symbol, FILE* out) {
    fprintf(out, EL_SV_FMT"#%u", EL_SV_FARG(symbol->name), symbol->id);
}

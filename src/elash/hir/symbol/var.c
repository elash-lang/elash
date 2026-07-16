#include <elash/hir/symbol/var.h>
#include <elash/hir/symbol.h>

ElHirSymbol* el_hir_new_var_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElHirType* type) {
    ElHirSymbol* sym = EL_DYNARENA_NEW(arena, ElHirSymbol);
    sym->name = name;
    sym->kind = EL_SYM_VAR;
    sym->id = id;
    sym->as.var = (ElHirVarSymbol) {
        .type = type,
    };
    return sym;
}

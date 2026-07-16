#include <elash/hir/symbol/type.h>
#include <elash/hir/symbol.h>

ElHirSymbol* el_hir_new_type_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElHirType* type) {
    ElHirSymbol* sym = EL_DYNARENA_NEW(arena, ElHirSymbol);
    sym->name = name;
    sym->kind = EL_SYM_TYPE;
    sym->id = id;
    sym->as.type = (ElHirTypeSymbol) {
        .type = type,
    };
    return sym;
}

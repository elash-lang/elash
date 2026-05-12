#include <elash/sema/symbol/type.h>
#include <elash/sema/symbol.h>

ElSymbol* el_sema_new_type_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElType* type) {
    ElSymbol* sym = EL_DYNARENA_NEW(arena, ElSymbol);
    sym->name = name;
    sym->kind = EL_SYM_TYPE;
    sym->id = id;
    sym->as.type = (ElTypeSymbol) {
        .type = type,
    };
    return sym;
}

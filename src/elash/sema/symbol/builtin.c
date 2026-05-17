#include <elash/sema/symbol/builtin.h>
#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

ElSymbol* el_sema_new_builtin_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElBuiltinKind kind
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElSymbol, {
        .name = name,
        .kind = EL_SYM_BUILTIN,
        .id = id,
        .as.builtin.kind = kind,
    });
}

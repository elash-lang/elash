#include <elash/hir/symbol/builtin.h>
#include <elash/hir/symbol.h>
#include <elash/util/dynarena.h>

ElHirSymbol* el_hir_new_builtin_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElBuiltinKind kind
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirSymbol, {
        .name = name,
        .kind = EL_SYM_BUILTIN,
        .id = id,
        .as.builtin.kind = kind,
    });
}

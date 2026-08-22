#include <elash/pp/symbol.h>

ElPpSymbol* _el_pp_new_sym_var(
    ElDynArena* arena, ElStringView name, ElSourceSpan defspan,
    ElPpValue* value, bool mut
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpSymbol, {
        .name = name,
        .kind = EL_PP_SYM_VAR,
        .defspan = defspan,
        .as.var = {
            .v = value,
            .is_mutable = mut,
            .was_mutated = false,
        },
    });
}

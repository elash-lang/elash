#include <elash/pp/symbol.h>
#include <elash/util/assert.h>

ElPpSymbol* _el_pp_new_sym_var(
    ElDynArena* arena, ElStringView name, ElSourceSpan defspan,
    ElPpValue* value, bool mut, bool is_public
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpSymbol, {
        .name = name,
        .kind = EL_PP_SYM_VAR,
        .defspan = defspan,
        .is_public = is_public,
        .as.var = {
            .v = value,
            .is_mutable = mut,
            .was_mutated = false,
        },
    });
}

ElPpSymbol* _el_pp_new_sym_func_impl(
    ElDynArena* arena, ElStringView name,
    ElSourceSpan defspan, bool is_public,
    ElPpFuncSym func
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpSymbol, {
        .name = name,
        .kind = EL_PP_SYM_FUNC,
        .defspan   = defspan,
        .is_public = is_public,
        .as.func = func,
    });
}

ElStringView _el_pp_sym_kind_to_string(ElPpSymbol* sym) {
    switch (sym->kind) {
    case EL_PP_SYM_FUNC:
        return EL_SV("function");

    case EL_PP_SYM_VAR:
        if (sym->as.var.is_mutable) {
            return EL_SV("variable");
        } else {
            return EL_SV("constant");
        }
    }

    EL_UNREACHABLE_ENUM_VAL(ElPpSymbolKind, sym->kind);

}

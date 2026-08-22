#pragma once
#include <elash/pp/value.h>

typedef enum ElPpSymbolKind {
    EL_PP_SYM_VAR,
    //EL_PP_SYM_FUNC,
    //EL_PP_SYM_MACRO,
} ElPpSymbolKind;

typedef struct ElPpVarSym {
    ElPpValue* v;
    bool is_mutable;
    bool was_mutated;
} ElPpVarSym;

typedef struct ElPpSymbol {
    ElStringView   name;
    ElPpSymbolKind kind;
    ElSourceSpan defspan;
    union {
        ElPpVarSym var;
    } as;
} ElPpSymbol;

ElPpSymbol* _el_pp_new_sym_var(
    ElDynArena* arena, ElStringView name, ElSourceSpan defspan,
    ElPpValue* value, bool mut
);

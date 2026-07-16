#pragma once

#include <elash/defs/sv.h>
#include <elash/defs/int-types.h>

typedef enum ElBuiltinKind {
    EL_BUILTIN_LEN,
    EL_BUILTIN_MKSLICE,
} ElBuiltinKind;

typedef struct ElHirBuiltinSymbol {
    ElBuiltinKind kind;
} ElHirBuiltinSymbol;

typedef struct ElHirSymbol ElHirSymbol;
typedef struct ElDynArena ElDynArena;

ElHirSymbol* el_hir_new_builtin_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElBuiltinKind kind
);

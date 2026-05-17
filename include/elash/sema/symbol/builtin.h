#pragma once

#include <elash/defs/sv.h>
#include <elash/defs/int-types.h>

typedef enum ElBuiltinKind {
    EL_BUILTIN_LEN,
} ElBuiltinKind;

typedef struct ElBuiltinSymbol {
    ElBuiltinKind kind;
} ElBuiltinSymbol;

typedef struct ElSymbol ElSymbol;
typedef struct ElDynArena ElDynArena;

ElSymbol* el_sema_new_builtin_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElBuiltinKind kind
);

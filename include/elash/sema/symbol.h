#pragma once

#include <elash/defs/sv.h>

#include "symbol/var.h"
#include "symbol/func.h"
#include "symbol/type.h"
#include "symbol/builtin.h"

typedef enum ElSymbolKind {
    EL_SYM_VAR,
    EL_SYM_FUNC,
    EL_SYM_TYPE,
    EL_SYM_BUILTIN,
} ElSymbolKind;

typedef struct ElSymbol {
    ElStringView name;
    ElSymbolKind kind;
    uint32_t id;
    union {
        ElVarSymbol var;
        ElFuncSymbol func;
        ElTypeSymbol type;
        ElBuiltinSymbol builtin;
    } as;
} ElSymbol;

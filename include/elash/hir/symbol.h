#pragma once

#include <elash/defs/sv.h>

#include "symbol/var.h"
#include "symbol/func.h"
#include "symbol/type.h"
#include "symbol/builtin.h"

typedef enum ElHirSymbolKind {
    EL_SYM_VAR,
    EL_SYM_FUNC,
    EL_SYM_TYPE,
    EL_SYM_BUILTIN,
} ElHirSymbolKind;

typedef struct ElHirSymbol {
    ElStringView name;
    ElHirSymbolKind kind;
    uint32_t id;
    union {
        ElHirVarSymbol     var;
        ElHirFuncSymbol    func;
        ElHirTypeSymbol    type;
        ElHirBuiltinSymbol builtin;
    } as;
} ElHirSymbol;

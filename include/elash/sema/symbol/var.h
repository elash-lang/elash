#pragma once

#include <elash/sema/type.h>
#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>

typedef struct ElSymbol ElSymbol;

typedef struct ElVarSymbol {
    ElType* type;
} ElVarSymbol;

ElSymbol* el_sema_new_var_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElType* type);

#pragma once

#include <elash/defs/int-types.h>
#include <elash/sema/type.h>
#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>

typedef struct ElSymbol ElSymbol;

typedef struct ElFuncSymbol {
    ElType* type;
    ElType* ret_type;
    ElSymbol** params;
    usize param_count;
} ElFuncSymbol;

ElSymbol* el_sema_new_func_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElType* ret_type, ElSymbol** params, usize param_count
);

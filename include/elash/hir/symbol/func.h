#pragma once

#include <elash/defs/int-types.h>
#include <elash/hir/type.h>
#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/sv.h>

typedef struct ElHirSymbol ElHirSymbol;

typedef struct ElHirFuncSymbol {
    ElHirType* type;
    ElHirSymbol** params;
    usize param_count;
    bool is_defined;
} ElHirFuncSymbol;

ElHirSymbol* el_hir_new_func_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElHirType* ret_type, ElHirSymbol** params, usize param_count
);

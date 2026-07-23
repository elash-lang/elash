#pragma once

#include <elash/hir/type.h>
#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/sv.h>

typedef struct ElHirSymbol ElHirSymbol;

typedef struct ElHirVarSymbol {
    ElHirType* type;
} ElHirVarSymbol;

ElHirSymbol* el_hir_new_var_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElHirType* type);

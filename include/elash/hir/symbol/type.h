#pragma once

#include <elash/hir/type.h>
#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>

typedef struct ElHirSymbol ElHirSymbol;

typedef struct ElHirTypeSymbol {
    ElHirType* type;
} ElHirTypeSymbol;

ElHirSymbol* el_hir_new_type_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElHirType* type);

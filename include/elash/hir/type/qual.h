#pragma once

#include <elash/sema/mutability.h>
#include <elash/util/dynarena.h>

typedef struct ElHirType ElHirType;

typedef struct ElHirQualType {
    ElHirType*       base; ///< cannot be another qual type, must be canonical
    ElMutabilitySpec mut;
} ElHirQualType;

ElHirType* el_hir_new_qual_type(ElDynArena* arena, ElHirType* base, ElMutabilitySpec mut);

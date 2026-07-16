#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirType ElHirType;

typedef struct ElHirRefType {
    ElHirType* base;
} ElHirRefType;

ElHirType* el_hir_new_ref_type(ElDynArena* arena, ElHirType* base);

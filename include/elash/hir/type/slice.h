#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirType ElHirType;
typedef struct ElHirSliceType {
    ElHirType* base;
} ElHirSliceType;

ElHirType* el_hir_new_slice_type(ElDynArena* arena, ElHirType* base);

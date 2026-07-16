#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirType ElHirType;
typedef struct ElHirRawSliceType {
    ElHirType* base;
} ElHirRawSliceType;

ElHirType* el_hir_new_raw_slice_type(ElDynArena* arena, ElHirType* base);

#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirType ElHirType;
typedef struct ElHirSliceType {
    ElHirType* base;
} ElHirSliceType;

ElHirType* el_hir_new_slice_type(ElDynArena* arena, ElHirType* base);

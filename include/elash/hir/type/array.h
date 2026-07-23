#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/int-types.h>

typedef struct ElHirType ElHirType;

typedef struct ElHirArrayType {
    ElHirType* base;
    usize size;
} ElHirArrayType;

ElHirType* el_hir_new_array_type(ElDynArena* arena, ElHirType* base, usize size);

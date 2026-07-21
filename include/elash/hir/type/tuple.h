#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirType ElHirType;
typedef struct ElHirTupleType {
    ElHirType** elements;
    usize count;
} ElHirTupleType;

ElHirType* el_hir_new_tuple_type(ElDynArena* arena, ElHirType** elements, usize count);

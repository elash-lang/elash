#pragma once

#include <elash/util/dynarena.h>

typedef struct ElType ElType;
typedef struct ElSliceType {
    ElType* base;
} ElSliceType;

ElType* el_sema_new_slice_type(ElDynArena* arena, ElType* base);

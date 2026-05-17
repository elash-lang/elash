#pragma once

#include <elash/util/dynarena.h>

typedef struct ElType ElType;
typedef struct ElRawSliceType {
    ElType* base;
} ElRawSliceType;

ElType* el_sema_new_raw_slice_type(ElDynArena* arena, ElType* base);


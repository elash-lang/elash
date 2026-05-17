#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElType ElType;

typedef struct ElArrayType {
    ElType* base;
    usize size;
} ElArrayType;

ElType* el_sema_new_array_type(ElDynArena* arena, ElType* base, usize size);

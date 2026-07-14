#pragma once

#include <elash/util/dynarena.h>

typedef struct ElType ElType;

typedef struct ElPointerType {
    ElType* base;
} ElPointerType;

ElType* el_sema_new_ref_type(ElDynArena* arena, ElType* base);

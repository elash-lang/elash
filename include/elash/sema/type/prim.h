#pragma once

#include <elash/util/dynarena.h>

typedef struct ElType ElType;

typedef enum ElPrimitiveTypeKind {
    EL_PRIMTYPE_VOID,
    // TODO: add more integer types
    EL_PRIMTYPE_INT,
    EL_PRIMTYPE_UINT,
    EL_PRIMTYPE_CHAR,
    EL_PRIMTYPE_BOOL,
} ElPrimitiveTypeKind;

typedef struct ElPrimitiveType {
    ElPrimitiveTypeKind kind;
} ElPrimitiveType;

ElType* el_sema_new_prim_type(ElDynArena* arena, ElPrimitiveTypeKind kind);

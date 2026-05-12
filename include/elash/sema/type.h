#pragma once

#include <stdio.h>

#include "type/prim.h"
#include "type/ptr.h"
#include "type/func.h"

typedef enum ElTypeKind {
    EL_TYPE_PRIM,
    EL_TYPE_PTR,
    EL_TYPE_FUNC,
} ElTypeKind;

typedef struct ElType ElType;
struct ElType {
    ElTypeKind kind;
    union {
        ElPrimitiveType prim;
        ElPointerType ptr;
        ElFunctionType func;
    } as;
};

void el_sema_dump_type(const ElType* type, FILE* out);
bool el_sema_type_eql(const ElType* lhs, const ElType* rhs);

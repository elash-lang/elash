#pragma once

#include <elash/util/strbuf.h>
#include <stdio.h>

#include "type/prim.h"
#include "type/ptr.h"
#include "type/func.h"
#include "type/array.h"
#include "type/slice.h"
#include "type/raw-slice.h"

typedef enum ElTypeKind {
    EL_TYPE_PRIM,
    EL_TYPE_PTR,
    EL_TYPE_FUNC,
    EL_TYPE_ARRAY,
    EL_TYPE_SLICE,
    EL_TYPE_RAW_SLICE,
} ElTypeKind;

typedef struct ElType ElType;
struct ElType {
    ElTypeKind kind;
    union {
        ElPrimitiveType prim;
        ElPointerType ptr;
        ElSliceType slice;
        ElArrayType array;
        ElRawSliceType raw_slice;
        ElFunctionType func;
    } as;
};

void el_sema_format_type_internal(const ElType* type, void (*write)(const char*, void*), void* ctx);

void el_sema_dump_type(const ElType* type, FILE* out);
void el_sema_format_type(const ElType* type, ElStringBuf* sb);

bool el_sema_type_eql(const ElType* lhs, const ElType* rhs);

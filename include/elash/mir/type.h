#pragma once

#include <elash/util/dynarena.h>
#include <elash/util/strbuf.h>
#include <elash/defs/int-types.h>

#include <stdbool.h>
#include <stdio.h>

#include "type/int.h"
#include "type/ptr.h"
#include "type/void.h"
#include "type/func.h"
#include "type/array.h"
#include "type/tuple.h"

typedef enum ElMirTypeKind {
    EL_MIR_TYPE_VOID,
    EL_MIR_TYPE_INT,
    EL_MIR_TYPE_PTR,
    EL_MIR_TYPE_FUNC,
    EL_MIR_TYPE_ARRAY,
    EL_MIR_TYPE_TUPLE,
} ElMirTypeKind;

struct ElMirType {
    ElMirTypeKind kind;
    union {
        ElMirIntType   integer;
        ElMirPtrType   ptr;
        ElMirFuncType  func;
        ElMirArrayType array;
        ElMirTupleType tuple;
    } as;
};

void el_mir_format_type_internal(const ElMirType* type, void (*write)(const char*, void*), void* ctx);
void el_mir_dump_type(const ElMirType* type, FILE* out);
void el_mir_format_type(const ElMirType* type, ElStringBuf* sb);
bool el_mir_type_eql(const ElMirType* lhs, const ElMirType* rhs);

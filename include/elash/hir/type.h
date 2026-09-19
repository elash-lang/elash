#pragma once

#include <elash/util/strbuf.h>
#include <stdio.h>

#include "type/opt.h"
#include "type/ref.h"
#include "type/prim.h"
#include "type/func.h"
#include "type/array.h"
#include "type/slice.h"
#include "type/tuple.h"
#include "type/struct.h"
#include "type/distinct.h"
#include "type/raw-slice.h"

typedef enum ElHirTypeKind {
    EL_HIR_TYPE_PRIM,
    EL_HIR_TYPE_REF,
    EL_HIR_TYPE_OPT,
    EL_HIR_TYPE_FUNC,
    EL_HIR_TYPE_ARRAY,
    EL_HIR_TYPE_SLICE,
    EL_HIR_TYPE_RWSLICE,
    EL_HIR_TYPE_STRUCT,
    EL_HIR_TYPE_TUPLE,
    EL_HIR_TYPE_DISTINCT,
} ElHirTypeKind;

typedef struct ElHirType ElHirType;
struct ElHirType {
    ElHirTypeKind kind;
    union {
        ElHirPrimType     prim;
        ElHirRefType      ref;
        ElHirOptType      opt;
        ElHirSliceType    slice;
        ElHirArrayType    array;
        ElHirRawSliceType rwslice;
        ElHirStructType   struct_;
        ElHirTupleType    tuple;
        ElHirFuncType     func;
        ElHirDistinctType distinct;
    } as;
};

void el_format_type_internal(const ElHirType* type, void (*write)(const char*, void*), void* ctx);

void el_hir_dump_type(const ElHirType* type, FILE* out);
void el_format_type(const ElHirType* type, ElStringBuf* sb);

bool el_hir_type_eql(const ElHirType* lhs, const ElHirType* rhs);
uhash el_hir_type_hash(const ElHirType* type);

ElHirType* el_hir_type_unwrap_distinct(ElHirType* type);
bool el_hir_type_is_incomplete(const ElHirType* type);

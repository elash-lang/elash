#pragma once

#include <elash/util/dynarena.h>
#include <elash/util/writer.h>
#include <elash/defs/int-types.h>

#include "type/int.h"
#include "type/float.h"
#include "type/ptr.h"
#include "type/void.h"
#include "type/func.h"
#include "type/array.h"
#include "type/tuple.h"

typedef enum ElMirTypeKind {
    EL_MIR_TYPE_VOID,
    EL_MIR_TYPE_INT,
    EL_MIR_TYPE_FLOAT,
    EL_MIR_TYPE_PTR,
    EL_MIR_TYPE_FUNC,
    EL_MIR_TYPE_ARRAY,
    EL_MIR_TYPE_TUPLE,
} ElMirTypeKind;

struct ElMirType {
    ElMirTypeKind kind;
    union {
        ElMirIntType   integer;
        ElMirFloatType float_;
        ElMirPtrType   ptr;
        ElMirFuncType  func;
        ElMirArrayType array;
        ElMirTupleType tuple;
    } as;
};

void el_mir_dump_type(const ElMirType* type, ElWriter out);

bool el_mir_type_eql(const ElMirType* lhs, const ElMirType* rhs);
uhash el_mir_type_hash(const ElMirType* type);

#pragma once

#include <elash/defs/sv.h>
#include <elash/source/span.h>
#include <elash/util/dynarena.h>
#include <elash/sema/mutability.h>

#include "common/ident.h"

#include "type/struct.h"
#include "type/tuple.h"
#include "type/slice.h"
#include "type/array.h"
#include "type/ref.h"
#include "type/opt.h"

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef enum ElAstTypeKind {
    EL_AST_TYPE_NAME,
    EL_AST_TYPE_REF,
    EL_AST_TYPE_OPT,
    EL_AST_TYPE_ARRAY,
    EL_AST_TYPE_SLICE,
    EL_AST_TYPE_STRUCT,
    EL_AST_TYPE_TUPLE,
} ElAstTypeKind;

struct ElAstType {
    ElAstTypeKind kind;
    ElSourceSpan  span;

    ElMutabilitySpec mut;
    union {
        ElAstIdent*     name;
        ElAstRefType    ref;
        ElAstOptType    opt;
        ElAstArrayType  array;
        ElAstSliceType  slice;
        ElAstStructType struct_;
        ElAstTupleType  tuple;
    } as;

    ElAstType* next;
};

ElAstType* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstIdent* name);
void el_ast_type_list_append(ElAstType** head, ElAstType** tail, ElAstType* type);

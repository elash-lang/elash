#pragma once

#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

#include "common/ident.h"

#include "type/slice.h"
#include "type/array.h"
#include "type/ref.h"

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef enum ElAstTypeKind {
    EL_AST_TYPE_NAME,
    EL_AST_TYPE_REF,
    EL_AST_TYPE_ARRAY,
    EL_AST_TYPE_SLICE,
} ElAstTypeKind;

struct ElAstType {
    ElAstTypeKind kind;
    ElSourceSpan span;
    union {
        ElAstIdent*    name;
        ElAstRefType   ref;
        ElAstArrayType array;
        ElAstSliceType slice;
    } as;
};

ElAstType* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdent* name);

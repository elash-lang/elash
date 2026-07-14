#pragma once

#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

#include "ident.h"

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef enum ElAstTypeKind {
    EL_AST_TYPE_NAME,
    EL_AST_TYPE_REF,
    EL_AST_TYPE_ARRAY,
    EL_AST_TYPE_SLICE,
    EL_AST_TYPE_RAW_SLICE,
} ElAstTypeKind;

typedef struct ElAstArrayType {
    ElAstType* base;
    ElAstExpr* size;
} ElAstArrayType;

struct ElAstType {
    ElAstTypeKind kind;
    ElSourceSpan span;
    union {
        ElAstIdent* name; // for EL_AST_TYPE_NAME
        ElAstType* base;  // for EL_AST_TYPE_REF, EL_AST_TYPE_SLICE, EL_AST_TYPE_RAW_SLICE
        ElAstArrayType array; // for EL_AST_TYPE_ARRAY
    };
};

ElAstType* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdent* name);
ElAstType* el_ast_new_type_ref(ElDynArena* arena, ElSourceSpan span, ElAstType* base);
ElAstType* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElAstType* base);
ElAstType* el_ast_new_type_raw_slice(ElDynArena* arena, ElSourceSpan span, ElAstType* base);
ElAstType* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElAstType* base, ElAstExpr* size);

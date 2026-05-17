#pragma once

#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

#include "ident.h"

typedef struct ElAstExprNode ElAstExprNode;
typedef struct ElAstTypeNode ElAstTypeNode;

typedef enum ElAstTypeKind {
    EL_AST_TYPE_NAME,
    EL_AST_TYPE_PTR,
    EL_AST_TYPE_ARRAY,
    EL_AST_TYPE_SLICE,
    EL_AST_TYPE_RAW_SLICE,
} ElAstTypeKind;

typedef struct ElAstArrayType {
    ElAstTypeNode* base;
    ElAstExprNode* size;
} ElAstArrayType;

struct ElAstTypeNode {
    ElAstTypeKind kind;
    ElSourceSpan span;
    union {
        ElAstIdentNode* name; // for EL_AST_TYPE_NAME
        ElAstTypeNode* base;  // for EL_AST_TYPE_PTR, EL_AST_TYPE_SLICE, EL_AST_TYPE_RAW_SLICE
        ElAstArrayType array; // for EL_AST_TYPE_ARRAY
    };
};

ElAstTypeNode* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdentNode* name);
ElAstTypeNode* el_ast_new_type_ptr(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base);
ElAstTypeNode* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base);
ElAstTypeNode* el_ast_new_type_raw_slice(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base);
ElAstTypeNode* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base, ElAstExprNode* size);

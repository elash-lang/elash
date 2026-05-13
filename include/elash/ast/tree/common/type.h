#pragma once

#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

#include "ident.h"

typedef struct ElAstExprNode ElAstExprNode;

typedef enum ElAstTypeKind {
    EL_AST_TYPE_NAME,
    EL_AST_TYPE_PTR,
} ElAstTypeKind;

typedef struct ElAstTypeNode ElAstTypeNode;
struct ElAstTypeNode {
    ElAstTypeKind kind;
    ElSourceSpan span;
    union {
        ElAstIdentNode* name; // for EL_AST_TYPE_NAME
        ElAstTypeNode* base;  // for EL_AST_TYPE_PTR
    };
};

ElAstTypeNode el_ast_type_name(ElSourceSpan span, ElAstIdentNode* name);
ElAstTypeNode el_ast_type_ptr(ElSourceSpan span, ElAstTypeNode* base);

ElAstTypeNode* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdentNode* name);
ElAstTypeNode* el_ast_new_type_ptr(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base);

void el_ast_dump_type(ElAstTypeNode* node, FILE* out);

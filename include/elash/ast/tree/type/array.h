#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/srcdoc.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef struct ElAstArrayType {
    ElAstType* base;
    ElAstExpr* size;
} ElAstArrayType;

ElAstType* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElAstType* base, ElAstExpr* size);

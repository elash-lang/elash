#pragma once

#include <elash/sema/mutability.h>
#include <elash/util/dynarena.h>
#include <elash/source/doc.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef struct ElAstArrayType {
    ElAstType* base;
    ElAstExpr* size;
} ElAstArrayType;

ElAstType* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstType* base, ElAstExpr* size);

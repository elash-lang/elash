#pragma once

#include <elash/sema/mutability.h>
#include <elash/util/dynarena.h>
#include <elash/source/doc.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef struct ElAstOptType {
    ElAstType* base;
} ElAstOptType;

ElAstType* el_ast_new_type_opt(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstType* base);


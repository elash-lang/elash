#pragma once

#include <elash/sema/mutability.h>
#include <elash/util/dynarena.h>
#include <elash/source/doc.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef struct ElAstSliceType {
    ElAstType* base;
    bool is_raw;
} ElAstSliceType;

ElAstType* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstType* base, bool is_raw);

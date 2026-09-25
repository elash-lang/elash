#pragma once

#include <elash/sema/mutability.h>
#include <elash/util/dynarena.h>
#include <elash/source/doc.h>

typedef struct ElAstType ElAstType;

typedef struct ElAstTupleType {
    ElAstType* head;
    usize count;
} ElAstTupleType;

ElAstType* el_ast_new_type_tuple(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstType* head, usize count);

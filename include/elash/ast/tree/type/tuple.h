#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/srcdoc.h>

typedef struct ElAstType ElAstType;

typedef struct ElAstTupleType {
    ElAstType* head;
    usize count;
} ElAstTupleType;

ElAstType* el_ast_new_type_tuple(ElDynArena* arena, ElSourceSpan span, ElAstType* head, usize count);

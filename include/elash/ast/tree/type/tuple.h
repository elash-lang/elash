#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/srcdoc.h>

typedef struct ElAstType ElAstType;

typedef struct ElAstTupleElement {
    ElSourceSpan span;
    ElAstType* type;
    struct ElAstTupleElement* next;
} ElAstTupleElement;

typedef struct ElAstTupleType {
    ElAstTupleElement* head;
    usize count;
} ElAstTupleType;

ElAstType* el_ast_new_type_tuple(ElDynArena* arena, ElSourceSpan span, ElAstTupleElement* head, usize count);
void el_ast_tuple_add_elem(ElAstTupleElement** head, ElAstTupleElement** tail, ElAstTupleElement* element);

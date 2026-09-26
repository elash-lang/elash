#pragma once

#include <elash/util/dynarena.h>
#include <elash/source/span.h>

#include <elash/ast/tree/toe.h>

typedef struct ElAstTypedefElem ElAstTypedefElem;
struct ElAstTypedefElem {
    ElStringView name;
    ElAstType* target;
    ElAstTypedefElem* next;
};

typedef struct ElAstDecl ElAstDecl;
typedef struct ElAstTypedef {
    ElAstTypedefElem* elements;
} ElAstTypedef;

ElAstTypedefElem* el_ast_new_typedef_elem(ElDynArena* arena, ElStringView name, ElAstType* target);
void el_ast_append_typedef_elem(ElAstTypedefElem** head, ElAstTypedefElem** tail, ElAstTypedefElem* elem);

ElAstDecl* el_ast_new_typedef(ElDynArena* arena, ElSourceSpan span, ElAstTypedefElem* elements);

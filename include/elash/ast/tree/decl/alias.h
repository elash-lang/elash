#pragma once

#include <elash/util/dynarena.h>
#include <elash/source/span.h>

#include <elash/ast/tree/toe.h>

typedef struct ElAstAliasElem ElAstAliasElem;
struct ElAstAliasElem {
    ElStringView name;
    ElAstToE* target;
    ElAstAliasElem* next;
};

typedef struct ElAstDecl ElAstDecl;
typedef struct ElAstAlias {
    ElAstAliasElem* elements;
} ElAstAlias;

ElAstAliasElem* el_ast_new_alias_elem(ElDynArena* arena, ElStringView name, ElAstToE* target);
void el_ast_append_alias_elem(ElAstAliasElem** head, ElAstAliasElem** tail, ElAstAliasElem* elem);

ElAstDecl* el_ast_new_alias(ElDynArena* arena, ElSourceSpan span, ElAstAliasElem* elements);

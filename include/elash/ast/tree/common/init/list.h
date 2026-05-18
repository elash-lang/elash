#pragma once

#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElAstInit ElAstInit;

typedef struct ElAstInitList {
    ElAstInit* head;
    usize count;
} ElAstInitList;

ElAstInit* el_ast_new_init_list(ElDynArena* arena, ElSourceSpan span, ElAstInit* head, usize count);
void el_ast_init_list_append(ElAstInit** head, ElAstInit** tail, ElAstInit* init);

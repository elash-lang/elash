#pragma once

#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElAstInitializer ElAstInitializer;

typedef struct ElAstInitList {
    struct ElAstInitializer* head;
    usize count;
} ElAstInitList;

ElAstInitializer* el_ast_new_init_list(ElDynArena* arena, ElSourceSpan span, ElAstInitializer* head, usize count);
void el_ast_init_list_append(ElAstInitializer** head, ElAstInitializer** tail, ElAstInitializer* init);

#pragma once

#include <elash/srcdoc/span.h>
#include "toplevel.h"

typedef struct ElAstModule {
    ElSourceSpan span;
    ElAstTopLevel* head;
    ElAstTopLevel* tail;
    usize count;
} ElAstModule;

ElAstModule* el_ast_new_module(ElDynArena* arena, ElSourceSpan span);
void el_ast_module_append(ElAstModule* module, ElAstTopLevel* node);

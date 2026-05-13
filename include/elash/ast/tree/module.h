#pragma once

#include <elash/srcdoc/span.h>
#include "toplevel.h"

typedef struct ElAstModuleNode {
    ElSourceSpan span;
    ElAstTopLevelNode* head;
    ElAstTopLevelNode* tail;
    usize count;
} ElAstModuleNode;

ElAstModuleNode el_ast_module(ElSourceSpan span);
ElAstModuleNode* el_ast_new_module(ElDynArena* arena, ElSourceSpan span);

void el_ast_module_append(ElAstModuleNode* module, ElAstTopLevelNode* node);


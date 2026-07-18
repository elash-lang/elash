#pragma once

#include <elash/srcdoc/span.h>
#include <elash/ast/tree/decl.h>

typedef struct ElAstModule {
    ElSourceSpan span;
    ElAstDecl* head;
    ElAstDecl* tail;
    usize count;
} ElAstModule;

ElAstModule* el_ast_new_module(ElDynArena* arena, ElSourceSpan span);
void el_ast_module_append(ElAstModule* module, ElAstDecl* node);

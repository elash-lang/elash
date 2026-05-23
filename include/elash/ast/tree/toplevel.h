#pragma once

#include <elash/srcdoc/span.h>

#include "common/decl.h"

typedef enum ElAstTopLevelType {
    EL_AST_TOPLVL_DECL,
} ElAstTopLevelType;

typedef struct ElAstTopLevel {
    ElAstTopLevelType type;
    ElSourceSpan span;
    union {
        ElAstDecl* decl;
    } as;
    ElAstTopLevel* next;
} ElAstTopLevel;

ElAstTopLevel* el_ast_new_toplevel_decl(ElDynArena* arena, ElAstDecl* decl);

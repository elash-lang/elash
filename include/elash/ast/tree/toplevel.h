#pragma once

#include <elash/srcdoc/span.h>

#include "toplevel/func-decl.h"
#include "toplevel/func-def.h"

typedef enum ElAstTopLevelType {
    EL_AST_TOPLVL_FUNC_DECL,
    EL_AST_TOPLVL_FUNC_DEF,
} ElAstTopLevelType;

typedef struct ElAstTopLevelNode {
    ElAstTopLevelType type;
    ElSourceSpan span;
    union {
        ElAstFuncDef  func_def;
        ElAstFuncDecl func_decl;
    } as;
    ElAstTopLevelNode* next;
} ElAstTopLevelNode;

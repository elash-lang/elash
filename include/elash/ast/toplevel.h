#pragma once

#include <elash/srcdoc/span.h>

#include "toplevel/func-def.h"

typedef enum ElAstTopLevelType {
    EL_AST_TOPLVL_FUNC_DEF,
} ElAstTopLevelType;

typedef struct ElAstTopLevelNode {
    ElAstTopLevelType type;
    ElSourceSpan span;
    union {
        ElAstFuncDef func_def;
    } as;
    ElAstTopLevelNode* next;
} ElAstTopLevelNode;

void el_ast_dump_toplevel(ElAstTopLevelNode* node, usize indent, FILE* out);

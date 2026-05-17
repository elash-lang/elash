#pragma once

#include <elash/ast/tree/common/init.h>
#include <elash/ast/tree/common/type.h>

typedef struct ElAstArrayLitNode {
    ElAstTypeNode* type;
    ElAstInitializer* init;
} ElAstArrayLitNode;

ElAstExprNode* el_ast_new_array_lit(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* type, ElAstInitializer* init);

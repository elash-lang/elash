#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/int-types.h>

typedef struct ElAstExprNode ElAstExprNode;

typedef struct ElAstCallExprNode {
    ElAstExprNode* callee;
    ElAstExprNode* args; // linked list
    usize arg_count;
} ElAstCallExprNode;

ElAstCallExprNode el_ast_call_expr(ElAstExprNode* callee, ElAstExprNode* args, usize arg_count);
ElAstExprNode* el_ast_new_call_expr(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* callee, ElAstExprNode* args, usize arg_count);

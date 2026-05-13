#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExprNode ElAstExprNode;
typedef struct ElAstStmtNode ElAstStmtNode;

typedef struct ElAstAssignStmtNode {
    ElAstExprNode* target;
    ElAstExprNode* value;
} ElAstAssignStmtNode;

ElAstStmtNode* el_ast_new_assign_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* target, ElAstExprNode* value);

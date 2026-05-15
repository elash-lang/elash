#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExprNode ElAstExprNode;
typedef struct ElAstStmtNode ElAstStmtNode;

typedef struct ElAstWhileStmtNode {
    ElAstExprNode* cond;
    ElAstStmtNode* body;
} ElAstWhileStmtNode;

ElAstStmtNode* el_ast_new_while_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExprNode* cond, ElAstStmtNode* body 
);


#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExprNode ElAstExprNode;
typedef struct ElAstStmtNode ElAstStmtNode;

typedef struct ElAstIfStmtNode {
    ElAstExprNode* cond;
    ElAstStmtNode* then;
    ElAstStmtNode* else_; // nullable
} ElAstIfStmtNode;

ElAstStmtNode* el_ast_new_if_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExprNode* cond, ElAstStmtNode* then, ElAstStmtNode* else_
);

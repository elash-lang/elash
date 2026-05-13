#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExprNode ElAstExprNode;
typedef struct ElAstStmtNode ElAstStmtNode;

typedef struct ElAstReturnStmtNode {
    ElAstExprNode* value; // nullable
} ElAstReturnStmtNode;

ElAstReturnStmtNode el_ast_return_stmt(ElAstExprNode* value);
ElAstStmtNode* el_ast_new_return_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* value);

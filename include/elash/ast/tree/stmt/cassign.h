#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/sema/expr/bin-op.h>

typedef struct ElAstExprNode ElAstExprNode;
typedef struct ElAstStmtNode ElAstStmtNode;

typedef struct ElAstCompoundAssignStmtNode {
    ElSemaBinOp op;
    ElAstExprNode* target;
    ElAstExprNode* value;
} ElAstCompoundAssignStmtNode;

ElAstStmtNode* el_ast_new_compound_assign_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElSemaBinOp op, ElAstExprNode* target, ElAstExprNode* value
);


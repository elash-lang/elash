#pragma once

#include <elash/sema/expr/bin-op.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExprNode ElHirExprNode;
typedef struct ElHirStmtNode ElHirStmtNode;

typedef struct ElHirCompoundAssignStmtNode {
    ElSemaBinOp op;
    ElHirExprNode* target;
    ElHirExprNode* value;
} ElHirCompoundAssignStmtNode;

ElHirStmtNode* el_hir_new_compound_assign_stmt(
    ElDynArena* arena, ElSemaBinOp op, ElHirExprNode* target, ElHirExprNode* value
);


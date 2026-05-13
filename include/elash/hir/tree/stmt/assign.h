#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmtNode ElHirStmtNode;
typedef struct ElHirExprNode ElHirExprNode;

typedef struct ElHirAssignStmtNode {
    ElHirExprNode* target;
    ElHirExprNode* value;
} ElHirAssignStmtNode;

ElHirStmtNode* el_hir_new_assign_stmt(ElDynArena* arena, ElHirExprNode* target, ElHirExprNode* value);

#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmtNode ElHirStmtNode;
typedef struct ElHirExprNode ElHirExprNode;

typedef struct ElHirWhileStmtNode {
    ElHirExprNode* cond;
    ElHirStmtNode* body;
} ElHirWhileStmtNode;

ElHirStmtNode* el_hir_new_while_stmt(ElDynArena* arena, ElHirExprNode* cond, ElHirStmtNode* body);


#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmtNode ElHirStmtNode;
typedef struct ElHirExprNode ElHirExprNode;

typedef struct ElHirIfStmtNode {
    ElHirExprNode* cond;
    ElHirStmtNode* then;
    ElHirStmtNode* else_; // nullable
} ElHirIfStmtNode;

ElHirStmtNode* el_hir_new_if_stmt(ElDynArena* arena, ElHirExprNode* cond, ElHirStmtNode* then, ElHirStmtNode* else_);


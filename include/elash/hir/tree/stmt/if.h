#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirIfStmt {
    ElHirExpr* cond;
    ElHirStmt* then;
    ElHirStmt* else_; // nullable
} ElHirIfStmt;

ElHirStmt* el_hir_new_if_stmt(ElDynArena* arena, ElHirExpr* cond, ElHirStmt* then, ElHirStmt* else_);

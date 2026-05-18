#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirWhileStmt {
    ElHirExpr* cond;
    ElHirStmt* body;
} ElHirWhileStmt;

ElHirStmt* el_hir_new_while_stmt(ElDynArena* arena, ElHirExpr* cond, ElHirStmt* body);

#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirAssignStmt {
    ElHirExpr* target;
    ElHirExpr* value;
} ElHirAssignStmt;

ElHirStmt* el_hir_new_assign_stmt(ElDynArena* arena, ElHirExpr* target, ElHirExpr* value);

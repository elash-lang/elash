#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirReturnStmt {
    ElHirExpr* value;
} ElHirReturnStmt;

ElHirStmt* el_hir_new_return_stmt(ElDynArena* arena, ElHirExpr* value);

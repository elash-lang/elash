#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirAssignStmt {
    ElHirExpr* target;
    ElHirExpr* value;
} ElHirAssignStmt;

ElHirStmt* el_hir_new_assign_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* target, ElHirExpr* value);

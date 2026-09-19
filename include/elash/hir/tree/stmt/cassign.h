#pragma once

#include <elash/sema/bin-op.h>
#include <elash/util/dynarena.h>
#include <elash/source/span.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirStmt ElHirStmt;

typedef struct ElHirCompoundAssignStmt {
    ElBinOp op;
    ElHirExpr* target;
    ElHirExpr* value;
} ElHirCompoundAssignStmt;

ElHirStmt* el_hir_new_compound_assign_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElBinOp op, ElHirExpr* target, ElHirExpr* value
);

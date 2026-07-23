#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirIfStmt {
    ElHirExpr* cond;
    ElHirStmt* then;
    ElHirStmt* else_; // nullable
} ElHirIfStmt;

ElHirStmt* el_hir_new_if_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* cond, ElHirStmt* then, ElHirStmt* else_);

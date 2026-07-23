#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirWhileStmt {
    ElHirExpr* cond;
    ElHirStmt* body;
} ElHirWhileStmt;

ElHirStmt* el_hir_new_while_stmt(ElDynArena* arena, ElSourceSpan span, ElHirExpr* cond, ElHirStmt* body);

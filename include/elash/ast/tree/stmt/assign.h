#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstAssignStmt {
    ElAstExpr* target;
    ElAstExpr* value;
} ElAstAssignStmt;

ElAstStmt* el_ast_new_assign_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* target, ElAstExpr* value);

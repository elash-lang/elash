#pragma once

#include <elash/util/dynarena.h>
#include <elash/source/span.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstWhileStmt {
    ElAstStmt* init; // nullable
    ElAstExpr* cond;
    ElAstStmt* body;
    ElAstStmt* post; // nullable
} ElAstWhileStmt;

ElAstStmt* el_ast_new_while_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstStmt* init, ElAstExpr* cond, ElAstStmt* body, ElAstStmt* post
);

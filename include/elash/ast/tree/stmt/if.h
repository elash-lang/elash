#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstIfStmt {
    ElAstExpr* cond;
    ElAstStmt* then;
    ElAstStmt* else_; // nullable
} ElAstIfStmt;

ElAstStmt* el_ast_new_if_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElAstExpr* cond, ElAstStmt* then, ElAstStmt* else_
);

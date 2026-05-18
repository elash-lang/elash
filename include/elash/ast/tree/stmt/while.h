#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstWhileStmt {
    ElAstExpr* cond;
    ElAstStmt* body;
} ElAstWhileStmt;

ElAstStmt* el_ast_new_while_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* cond, ElAstStmt* body);

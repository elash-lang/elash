#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstReturnStmt {
    ElAstExpr* value; // nullable
} ElAstReturnStmt;

ElAstReturnStmt el_ast_return_stmt(ElAstExpr* value);
ElAstStmt* el_ast_new_return_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* value);

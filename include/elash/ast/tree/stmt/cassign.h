#pragma once

#include <elash/util/dynarena.h>
#include <elash/source/span.h>

#include <elash/sema/bin-op.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstInit ElAstInit;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstCompoundAssignStmt {
    ElBinOp op;
    ElAstExpr* target;
    ElAstInit* value;
} ElAstCompoundAssignStmt;

ElAstStmt* el_ast_new_compound_assign_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElBinOp op, ElAstExpr* target, ElAstInit* value
);

#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/sema/expr/bin-op.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstInit ElAstInit;
typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstCompoundAssignStmt {
    ElSemaBinOp op;
    ElAstExpr* target;
    ElAstInit* value;
} ElAstCompoundAssignStmt;

ElAstStmt* el_ast_new_compound_assign_stmt(
    ElDynArena* arena, ElSourceSpan span,
    ElSemaBinOp op, ElAstExpr* target, ElAstInit* value
);

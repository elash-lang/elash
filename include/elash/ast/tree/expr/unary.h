#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>
#include <elash/source/span.h>

#include <elash/sema/unary-op.h>

typedef struct ElAstExpr ElAstExpr;

typedef ElUnaryOp ElAstUnaryOp;

typedef struct ElAstUnaryExpr {
    ElAstUnaryOp op;
    ElAstExpr* operand;
} ElAstUnaryExpr;

ElAstExpr* el_ast_new_unary_expr(ElDynArena* arena, ElSourceSpan span, ElAstUnaryOp op, ElAstExpr* operand);

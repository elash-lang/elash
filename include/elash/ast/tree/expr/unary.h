#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>

#include <elash/sema/expr/unary-op.h>

typedef struct ElAstExprNode ElAstExprNode;

typedef ElSemaUnaryOp ElAstUnaryOp;

typedef struct ElAstUnaryExprNode {
    ElAstUnaryOp op;
    ElAstExprNode* operand;
} ElAstUnaryExprNode;

ElAstExprNode* el_ast_new_unary_expr(ElDynArena* arena, ElSourceSpan span, ElAstUnaryOp op, ElAstExprNode* operand);


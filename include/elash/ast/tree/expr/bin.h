#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>

#include <elash/sema/expr/bin-op.h>

typedef struct ElAstExprNode ElAstExprNode;

typedef ElSemaBinOp ElAstBinOp;

typedef struct ElAstBinExprNode {
    ElAstExprNode* left;
    ElAstBinOp op;
    ElAstExprNode* right;
} ElAstBinExprNode;

ElAstExprNode* el_ast_new_bin_expr(ElDynArena* arena, ElSourceSpan span, ElAstBinOp op, ElAstExprNode* left, ElAstExprNode* right);

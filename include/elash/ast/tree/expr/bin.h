#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>

#include <elash/sema/bin-op.h>

typedef struct ElAstExpr ElAstExpr;

typedef ElSemaBinOp ElAstBinOp;

typedef struct ElAstBinExpr {
    ElAstExpr* left;
    ElAstBinOp op;
    ElAstExpr* right;
} ElAstBinExpr;

ElAstExpr* el_ast_new_bin_expr(ElDynArena* arena, ElSourceSpan span, ElAstBinOp op, ElAstExpr* left, ElAstExpr* right);

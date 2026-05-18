#pragma once

#include <elash/sema/expr/bin-op.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

typedef struct ElHirBinExpr {
    ElHirExpr* left;
    ElSemaBinOp op;
    ElHirExpr* right;
} ElHirBinExpr;

ElHirExpr* el_hir_new_bin_expr(ElDynArena* arena, ElType* type, ElSemaBinOp op, ElHirExpr* left, ElHirExpr* right);

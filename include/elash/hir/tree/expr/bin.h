#pragma once

#include <elash/sema/bin-op.h>
#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef struct ElHirBinExpr {
    ElHirExpr* left;
    ElSemaBinOp op;
    ElHirExpr* right;
} ElHirBinExpr;

ElHirExpr* el_hir_new_bin_expr(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElSemaBinOp op, ElHirExpr* left, ElHirExpr* right);

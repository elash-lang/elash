#pragma once

#include <elash/sema/unary-op.h>
#include <elash/util/dynarena.h>
#include <elash/source/span.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef struct ElHirUnaryExpr {
    ElUnaryOp op;
    ElHirExpr* operand;
} ElHirUnaryExpr;

ElHirExpr* el_hir_new_unary_expr(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElUnaryOp op, ElHirExpr* operand);

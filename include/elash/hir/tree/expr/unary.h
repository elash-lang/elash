#pragma once

#include <elash/sema/expr/unary-op.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

typedef struct ElHirUnaryExpr {
    ElSemaUnaryOp op;
    ElHirExpr* operand;
} ElHirUnaryExpr;

ElHirExpr* el_hir_new_unary_expr(ElDynArena* arena, ElType* type, ElSemaUnaryOp op, ElHirExpr* operand);

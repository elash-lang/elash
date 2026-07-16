#pragma once

#include <elash/sema/unary-op.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef struct ElHirUnaryExpr {
    ElSemaUnaryOp op;
    ElHirExpr* operand;
} ElHirUnaryExpr;

ElHirExpr* el_hir_new_unary_expr(ElDynArena* arena, ElHirType* type, ElSemaUnaryOp op, ElHirExpr* operand);

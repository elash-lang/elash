#pragma once

#include <elash/sema/type.h>
#include <elash/sema/symbol.h>
#include <elash/sema/const.h>
#include <elash/util/dynarena.h>

#include "expr/bin.h"
#include "expr/unary.h"
#include "expr/const.h"
#include "expr/call.h"
#include "expr/cast.h"
#include "expr/array-lit.h"
#include "expr/untyped-lit.h"

typedef enum ElHirExprKind {
    EL_HIR_EXPR_BINARY,
    EL_HIR_EXPR_UNARY,
    EL_HIR_EXPR_CONST,
    EL_HIR_EXPR_SYMBOL, // resolved identifier
    EL_HIR_EXPR_CALL,
    EL_HIR_EXPR_CAST,
    EL_HIR_EXPR_ARRAYLIT,
    EL_HIR_EXPR_UNTYPEDLIT,
} ElHirExprKind;

typedef struct ElHirExpr {
    ElHirExprKind kind;
    ElType* type; // NULL if it's untyped (i.e. kind == untyped lit)
    union {
        ElHirBinExpr    binary;
        ElHirUnaryExpr  unary;
        ElConstant      constant;
        ElSymbol*       symbol;
        ElHirCallExpr   call;
        ElHirCastExpr   cast;
        ElHirArrayLit   array_lit;
        ElHirUntypedLit untyped_lit;
    } as;
} ElHirExpr;

ElHirExpr* el_hir_new_symbol_expr(ElDynArena* arena, ElType* type, ElSymbol* symbol);

#pragma once

#include <elash/hir/type.h>
#include <elash/hir/symbol.h>
#include <elash/hir/const.h>
#include <elash/util/dynarena.h>
#include <elash/source/span.h>

#include "expr/bin.h"
#include "expr/unary.h"
#include "expr/const.h"
#include "expr/call.h"
#include "expr/intr.h"
#include "expr/cast.h"
#include "expr/member.h"
#include "expr/literal.h"
#include "expr/agginit.h"
#include "expr/strconst.h"

typedef enum ElHirExprKind {
    EL_HIR_EXPR_BINARY,
    EL_HIR_EXPR_UNARY,
    EL_HIR_EXPR_CONST,
    EL_HIR_EXPR_SYMBOL, // resolved identifier
    EL_HIR_EXPR_CALL,
    EL_HIR_EXPR_INTR,
    EL_HIR_EXPR_CAST,
    EL_HIR_EXPR_AGGINIT,
    EL_HIR_EXPR_STRCONST,
    EL_HIR_EXPR_LITERAL,
    EL_HIR_EXPR_MEMBER,
    EL_HIR_EXPR_TMEMBER,
} ElHirExprKind;

typedef struct ElHirExpr {
    ElHirExprKind kind;
    ElSourceSpan span;
    ElHirType* type; // NULL if it's untyped (i.e. kind == literal)
    union {
        ElHirBinExpr     binary;
        ElHirUnaryExpr   unary;
        ElHirConstant    constant;
        ElHirSymbol*     symbol;
        ElHirCallExpr    call;
        ElHirIntrExpr    intr;
        ElHirCastExpr    cast;
        ElHirAggInit     agginit;
        ElHirStringConst strconst;
        ElHirLiteral     literal;
        ElHirMemberExpr  member;
        ElHirTMemberExpr tmember;
    } as;
} ElHirExpr;

ElHirExpr* el_hir_new_symbol_expr(ElDynArena* arena, ElSourceSpan span, ElHirType* type, ElHirSymbol* symbol);
bool el_hir_expr_is_lvalue(const ElHirExpr* hir);

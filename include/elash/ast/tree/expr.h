#pragma once

#include "expr/bin.h"
#include "expr/unary.h"
#include "expr/literal.h"
#include "expr/array-lit.h"
#include "expr/call.h"
#include "expr/cast.h"

#include "common/ident.h"
#include "common/type.h"

#include <elash/defs/int-types.h>
#include <elash/srcdoc/span.h>

typedef enum ElAstExprType {
    EL_AST_EXPR_BINARY,
    EL_AST_EXPR_UNARY,
    EL_AST_EXPR_LITERAL,
    EL_AST_EXPR_ARRAYLIT,
    EL_AST_EXPR_IDENT,
    EL_AST_EXPR_CALL,
    EL_AST_EXPR_CAST,
} ElAstExprType;

typedef struct ElAstExpr {
    ElAstExprType type;
    ElSourceSpan span;
    union {
        ElAstBinExpr   binary;
        ElAstUnaryExpr unary;
        ElAstLiteral   literal;
        ElAstArrayLit  array_lit;
        ElAstIdent     ident;
        ElAstCallExpr  call;
        ElAstCastExpr  cast;
    } as;
    ElAstExpr* next;
} ElAstExpr;

void el_ast_expr_list_append(ElAstExpr** head, ElAstExpr** tail, ElAstExpr* expr);

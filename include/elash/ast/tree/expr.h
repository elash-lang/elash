#pragma once

#include "expr/bin.h"
#include "expr/unary.h"
#include "expr/literal.h"
#include "expr/array-lit.h"
#include "expr/call.h"

#include "common/ident.h"
#include "common/type.h"
#include "common/init.h"

#include <elash/defs/int-types.h>
#include <elash/srcdoc/span.h>

typedef enum ElAstExprType {
    EL_AST_EXPR_BINARY,
    EL_AST_EXPR_UNARY,
    EL_AST_EXPR_LITERAL,
    EL_AST_EXPR_IDENT,
    EL_AST_EXPR_CALL,
    EL_AST_EXPR_ARRAY_LITERAL,
} ElAstExprType;

typedef struct ElAstExprNode {
    ElAstExprType type;
    ElSourceSpan span;
    union {
        ElAstBinExprNode binary;
        ElAstUnaryExprNode unary;
        ElAstLiteralNode literal;
        ElAstArrayLitNode array_lit;
        ElAstIdentNode ident;
        ElAstCallExprNode call;
    } as;
    ElAstExprNode* next;
} ElAstExprNode;

void el_ast_expr_list_append(ElAstExprNode** head, ElAstExprNode** tail, ElAstExprNode* expr);

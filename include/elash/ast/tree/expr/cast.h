#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef struct ElAstCastExpr {
    ElAstExpr* expr;
    ElAstType* type;
} ElAstCastExpr;

ElAstExpr* el_ast_new_cast_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr, ElAstType* type);

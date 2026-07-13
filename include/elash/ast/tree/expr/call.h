#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/int-types.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstInit ElAstInit;

typedef struct ElAstCallExpr {
    ElAstExpr* callee;
    ElAstInit* args; // linked list
    usize arg_count;
} ElAstCallExpr;

ElAstExpr* el_ast_new_call_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* callee, ElAstInit* args, usize arg_count);

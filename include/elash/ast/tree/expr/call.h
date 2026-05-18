#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/int-types.h>

typedef struct ElAstExpr ElAstExpr;

typedef struct ElAstCallExpr {
    ElAstExpr* callee;
    ElAstExpr* args; // linked list
    usize arg_count;
} ElAstCallExpr;

ElAstExpr* el_ast_new_call_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* callee, ElAstExpr* args, usize arg_count);

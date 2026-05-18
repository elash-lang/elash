#pragma once

#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

#include "init/list.h"

typedef struct ElAstExpr ElAstExpr;

typedef enum ElAstInitKind {
    EL_AST_INIT_EXPR,
    EL_AST_INIT_LIST,
} ElAstInitKind;

typedef struct ElAstInit {
    ElAstInitKind kind;
    ElSourceSpan span;
    union {
        ElAstExpr* expr;
        ElAstInitList list;
    };
    struct ElAstInit* next;
} ElAstInit;

ElAstInit* el_ast_new_init_expr(ElDynArena* arena, ElAstExpr* expr);
ElAstInit* el_ast_new_init_list(ElDynArena* arena, ElSourceSpan span, ElAstInit* head, usize count);

void el_ast_init_list_append(ElAstInit** head, ElAstInit** tail, ElAstInit* init);

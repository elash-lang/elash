#pragma once

#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

#include "init/list.h"

typedef struct ElAstExprNode ElAstExprNode;

typedef enum ElAstInitializerKind {
    EL_AST_INIT_EXPR,
    EL_AST_INIT_LIST,
} ElAstInitializerKind;

typedef struct ElAstInitializer {
    ElAstInitializerKind kind;
    ElSourceSpan span;
    union {
        ElAstExprNode* expr;
        ElAstInitList list;
    };
    struct ElAstInitializer* next;
} ElAstInitializer;

ElAstInitializer* el_ast_new_init_expr(ElDynArena* arena, ElAstExprNode* expr);
ElAstInitializer* el_ast_new_init_list(ElDynArena* arena, ElSourceSpan span, ElAstInitializer* head, usize count);

void el_ast_init_list_append(ElAstInitializer** head, ElAstInitializer** tail, ElAstInitializer* init);

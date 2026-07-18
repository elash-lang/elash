#pragma once

#include <elash/ast/tree/type.h>
#include <elash/ast/tree/init.h>

typedef struct ElAstArrayLit {
    ElAstType* type;
    ElAstInit* init;
} ElAstArrayLit;

ElAstExpr* el_ast_new_array_lit(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstInit* init);

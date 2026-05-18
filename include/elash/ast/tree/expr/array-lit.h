#pragma once

#include <elash/ast/tree/common/init.h>
#include <elash/ast/tree/common/type.h>

typedef struct ElAstArrayLit {
    ElAstType* type;
    ElAstInit* init;
} ElAstArrayLit;

ElAstExpr* el_ast_new_array_lit(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstInit* init);

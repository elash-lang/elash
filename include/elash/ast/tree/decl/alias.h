#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/ast/tree/toe.h>

typedef struct ElAstDecl ElAstDecl;
typedef struct ElAstAlias {
    ElStringView name;
    ElAstToE target;
} ElAstAlias;

ElAstDecl* el_ast_new_alias(ElDynArena* arena, ElSourceSpan span, ElStringView name, ElAstToE target);

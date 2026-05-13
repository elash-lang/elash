#pragma once

#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

typedef struct ElAstExprNode ElAstExprNode;

typedef struct ElAstIdentNode {
    ElSourceSpan span;
    ElStringView name;
} ElAstIdentNode;

ElAstIdentNode el_ast_ident_node(ElSourceSpan span, ElStringView name);
ElAstIdentNode* el_ast_new_ident_node_raw(ElDynArena* arena, ElSourceSpan span, ElStringView name);
ElAstExprNode* el_ast_new_ident_node(ElDynArena* arena, ElSourceSpan span, ElStringView name);

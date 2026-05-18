#pragma once

#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

typedef struct ElAstExpr ElAstExpr;

typedef struct ElAstIdent {
    ElSourceSpan span;
    ElStringView name;
} ElAstIdent;

ElAstIdent el_ast_ident(ElSourceSpan span, ElStringView name);
ElAstIdent* el_ast_new_ident_raw(ElDynArena* arena, ElSourceSpan span, ElStringView name);
ElAstExpr* el_ast_new_ident(ElDynArena* arena, ElSourceSpan span, ElStringView name);

#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/type.h>

typedef struct ElAstDecl ElAstDecl;

typedef struct ElAstVarDecl {
    ElAstType*  type;
    ElAstIdent* name;
} ElAstVarDecl;

ElAstDecl* el_ast_new_var_decl(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name);

#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/common/type.h>

#include <elash/ast/tree/init.h>

typedef struct ElAstDecl ElAstDecl;

typedef struct ElAstVarDef {
    ElAstType*  type;
    ElAstIdent* name;
    ElAstInit*  init; // nullable
} ElAstVarDef;

ElAstDecl* el_ast_new_var_def(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name, ElAstInit* init);

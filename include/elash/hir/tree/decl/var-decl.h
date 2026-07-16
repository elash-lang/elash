#pragma once

#include <elash/hir/symbol.h>
#include <elash/util/dynarena.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirVarDecl {
    ElHirSymbol* var;
} ElHirVarDecl;

ElHirDecl* el_hir_new_var_decl(ElDynArena* arena, ElHirSymbol* symbol);

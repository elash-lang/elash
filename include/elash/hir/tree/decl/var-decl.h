#pragma once

#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirVarDecl {
    ElSymbol* var;
} ElHirVarDecl;

ElHirDecl* el_hir_new_var_decl(ElDynArena* arena, ElSymbol* symbol);

#pragma once

#include <elash/hir/symbol.h>
#include <elash/util/dynarena.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirFuncDecl {
    ElHirSymbol* symbol;
} ElHirFuncDecl;

ElHirDecl* el_hir_new_func_decl(ElDynArena* arena, ElHirSymbol* symbol);

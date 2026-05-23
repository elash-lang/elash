#pragma once

#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirFuncDecl {
    ElSymbol* symbol;
} ElHirFuncDecl;

ElHirDecl* el_hir_new_func_decl(ElDynArena* arena, ElSymbol* symbol);

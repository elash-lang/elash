#pragma once

#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

typedef struct ElHirTopLevel ElHirTopLevel;

typedef struct ElHirFuncDecl {
    ElSymbol* symbol;
} ElHirFuncDecl;

ElHirTopLevel* el_hir_new_func_decl(ElDynArena* arena, ElSymbol* symbol);

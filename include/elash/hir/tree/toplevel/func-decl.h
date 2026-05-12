#pragma once

#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

typedef struct ElHirTopLevelNode ElHirTopLevelNode;

typedef struct ElHirFuncDecl {
    ElSymbol* symbol;
} ElHirFuncDecl;

ElHirTopLevelNode* el_hir_new_func_decl(ElDynArena* arena, ElSymbol* symbol);

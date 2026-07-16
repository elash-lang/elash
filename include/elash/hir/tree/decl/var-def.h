#pragma once

#include <elash/hir/symbol.h>
#include <elash/hir/tree/expr.h>
#include <elash/util/dynarena.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirVarDef {
    ElHirSymbol* var;
    ElHirExpr*  init; // nullable
} ElHirVarDef;

ElHirDecl* el_hir_new_var_def(ElDynArena* arena, ElHirSymbol* sym, ElHirExpr* init);

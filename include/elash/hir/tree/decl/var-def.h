#pragma once

#include <elash/sema/symbol.h>
#include <elash/hir/tree/expr.h>
#include <elash/util/dynarena.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirVarDef {
    ElSymbol*  var;
    ElHirExpr* init; // nullable
} ElHirVarDef;

ElHirDecl* el_hir_new_var_def(ElDynArena* arena, ElSymbol* sym, ElHirExpr* init);

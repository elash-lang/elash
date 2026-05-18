#pragma once

#include <elash/sema/symbol.h>
#include <elash/hir/tree/expr.h>

typedef struct ElHirStmt ElHirStmt;

typedef struct ElHirVarDefStmt {
    ElSymbol*      var;
    ElHirExpr* init; // nullable
} ElHirVarDefStmt;

ElHirStmt* el_hir_new_var_def_stmt(ElDynArena* arena, ElSymbol* sym, ElHirExpr* init);

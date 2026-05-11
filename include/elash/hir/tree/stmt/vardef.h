#pragma once

#include <elash/sema/symbol.h>
#include <elash/hir/tree/expr.h>

typedef struct ElHirStmtNode ElHirStmtNode;

typedef struct ElHirVarDefStmtNode {
    ElSymbol*      var;
    ElHirExprNode* init; // nullable
} ElHirVarDefStmtNode;

ElHirStmtNode* el_hir_new_var_def_stmt(ElDynArena* arena, ElSymbol* sym, ElHirExprNode* init);

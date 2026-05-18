#pragma once

#include <elash/sema/type.h>
#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

#include "stmt/cassign.h"
#include "stmt/block.h"
#include "stmt/vardef.h"
#include "stmt/assign.h"
#include "stmt/return.h"

#include "stmt/if.h"
#include "stmt/while.h"

#include "stmt/break.h"
#include "stmt/continue.h"

typedef enum ElHirStmtKind {
    EL_HIR_STMT_EXPR,
    EL_HIR_STMT_RETURN,
    EL_HIR_STMT_COMPOUND_ASSIGN,
    EL_HIR_STMT_VAR_DEF,
    EL_HIR_STMT_ASSIGN,
    EL_HIR_STMT_BLOCK,
    EL_HIR_STMT_IF,
    EL_HIR_STMT_WHILE,
    EL_HIR_STMT_BREAK,
    EL_HIR_STMT_CONTINUE,
} ElHirStmtKind;

typedef struct ElHirStmt {
    ElHirStmtKind kind;
    union {
        ElHirExpr* expr;

        ElHirReturnStmt return_;
        ElHirVarDefStmt var_def;
        ElHirAssignStmt assign;

        ElHirCompoundAssignStmt cassign;

        ElHirBlockStmt block;
        ElHirWhileStmt while_;
        ElHirIfStmt    if_;

        ElHirContinueStmt continue_;
        ElHirBreakStmt    break_;
    } as;
    ElHirStmt* next;
} ElHirStmt;

ElHirStmt* el_hir_new_expr_stmt(ElDynArena* arena, ElHirExpr* expr);
void el_hir_stmt_list_append(ElHirStmt** head, ElHirStmt** tail, ElHirStmt* stmt);

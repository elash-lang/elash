#pragma once

#include <elash/sema/type.h>
#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

#include "stmt/if.h"
#include "stmt/while.h"
#include "stmt/block.h"
#include "stmt/vardef.h"
#include "stmt/assign.h"
#include "stmt/return.h"

typedef enum ElHirStmtKind {
    EL_HIR_STMT_EXPR,
    EL_HIR_STMT_RETURN,
    EL_HIR_STMT_VAR_DEF,
    EL_HIR_STMT_ASSIGN,
    EL_HIR_STMT_BLOCK,
    EL_HIR_STMT_IF,
    EL_HIR_STMT_WHILE,
} ElHirStmtKind;

typedef struct ElHirStmtNode {
    ElHirStmtKind kind;
    union {
        ElHirExprNode* expr;

        ElHirReturnStmtNode return_;
        ElHirVarDefStmtNode var_def;
        ElHirAssignStmtNode assign;
        
        ElHirBlockStmtNode block;
        ElHirWhileStmtNode while_;
        ElHirIfStmtNode    if_;
    } as;
    ElHirStmtNode* next;
} ElHirStmtNode;

ElHirStmtNode* el_hir_new_expr_stmt(ElDynArena* arena, ElHirExprNode* expr);
void el_hir_stmt_list_append(ElHirStmtNode** head, ElHirStmtNode** tail, ElHirStmtNode* stmt);

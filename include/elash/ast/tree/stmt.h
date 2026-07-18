#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include "expr.h"

#include "decl.h"
#include "common/block.h"

#include "stmt/cassign.h"
#include "stmt/assign.h"
#include "stmt/return.h"

#include "stmt/while.h"
#include "stmt/if.h"

#include "stmt/continue.h"
#include "stmt/break.h"

typedef enum ElAstStmtType {
    EL_AST_STMT_EXPR,
    EL_AST_STMT_RETURN,
    EL_AST_STMT_DECL,
    EL_AST_STMT_ASSIGN,
    EL_AST_STMT_BLOCK,
    EL_AST_STMT_COMPOUND_ASSIGN,
    EL_AST_STMT_IF,
    EL_AST_STMT_WHILE,
    EL_AST_STMT_BREAK,
    EL_AST_STMT_CONTINUE,
} ElAstStmtType;

typedef struct ElAstStmt {
    ElAstStmtType type;
    ElSourceSpan span;
    union {
        ElAstExpr* expr;

        ElAstBlockStmt  block;
        ElAstReturnStmt return_;
        ElAstDecl*      decl;
        ElAstAssignStmt assign;

        ElAstCompoundAssignStmt cassign;

        ElAstIfStmt    if_;
        ElAstWhileStmt while_;

        ElAstBreakStmt    break_;
        ElAstContinueStmt continue_;
    } as;
    ElAstStmt* next; // linked list; used in block stmt
} ElAstStmt;

ElAstStmt* el_ast_new_expr_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr);
ElAstStmt* el_ast_new_decl_stmt(ElDynArena* arena, ElSourceSpan span, ElAstDecl* decl);

void el_ast_stmt_list_append(ElAstStmt** head, ElAstStmt** tail, ElAstStmt* stmt);

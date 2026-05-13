#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include "expr.h"

#include "stmt/vardef.h"
#include "stmt/return.h"
#include "stmt/block.h"

#include "stmt/if.h"

typedef enum ElAstStmtType {
    EL_AST_STMT_EXPR,
    EL_AST_STMT_RETURN,
    EL_AST_STMT_VAR_DEF,
    EL_AST_STMT_BLOCK,
    EL_AST_STMT_IF,
} ElAstStmtType;

typedef struct ElAstStmtNode {
    ElAstStmtType type;
    ElSourceSpan span;
    union {
        ElAstExprNode* expr;

        ElAstBlockStmtNode block;
        ElAstReturnStmtNode return_;
        ElAstVarDefStmtNode var_def;

        ElAstIfStmtNode if_;
    } as;
    ElAstStmtNode* next; // linked list; used in block stmt
} ElAstStmtNode;

ElAstStmtNode el_ast_expr_stmt(ElSourceSpan span, ElAstExprNode* expr);
ElAstStmtNode* el_ast_new_expr_stmt(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* expr);

void el_ast_stmt_list_append(ElAstStmtNode** head, ElAstStmtNode** tail, ElAstStmtNode* stmt);

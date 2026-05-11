#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/ast/common/ident.h>
#include <elash/ast/common/type.h>
#include <elash/ast/expr.h>

typedef struct ElAstStmtNode ElAstStmtNode;

typedef struct ElAstVarDefStmtNode {
    ElAstTypeNode*  type;
    ElAstIdentNode* name;
    ElAstExprNode*  init; // nullable
} ElAstVarDefStmtNode;

ElAstStmtNode* el_ast_new_var_def_stmt(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* type, ElAstIdentNode* name, ElAstExprNode* init);

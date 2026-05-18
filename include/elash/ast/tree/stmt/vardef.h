#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/common/type.h>
#include <elash/ast/tree/common/init.h>

typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstVarDefStmt {
    ElAstType*  type;
    ElAstIdent* name;
    ElAstInit*  init; // nullable
} ElAstVarDefStmt;

ElAstStmt* el_ast_new_var_def_stmt(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name, ElAstInit* init);

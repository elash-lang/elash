#pragma once

#include <elash/srcdoc/span.h>

#include "decl/var-def.h"
#include "decl/func-def.h"
#include "decl/func-decl.h"

typedef enum ElAstDeclType {
    EL_AST_DECL_VAR,
    EL_AST_DECL_FUNC_DEF,
    EL_AST_DECL_FUNC_DECL,
} ElAstDeclType;

typedef struct ElAstDecl {
    ElAstDeclType type;
    ElSourceSpan span;
    union {
        ElAstVarDef var;
        ElAstFuncDef func_def;
        ElAstFuncDecl func_decl;
    } as;
} ElAstDecl;

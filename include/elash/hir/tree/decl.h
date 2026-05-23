#pragma once

#include <elash/sema/symbol.h>

#include "decl/var-def.h"
#include "decl/var-decl.h"
#include "decl/func-def.h"
#include "decl/func-decl.h"

typedef enum ElHirDeclKind {
    EL_HIR_DECL_VAR_DEF,
    EL_HIR_DECL_VAR_DECL,
    EL_HIR_DECL_FUNC_DEF,
    EL_HIR_DECL_FUNC_DECL,
} ElHirDeclKind;

typedef struct ElHirDecl {
    ElHirDeclKind kind;
    union {
        ElHirVarDef   var_def;
        ElHirVarDecl  var_decl;
        ElHirFuncDef  func_def;
        ElHirFuncDecl func_decl;
    } as;
} ElHirDecl;

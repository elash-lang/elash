#pragma once

#include "toplevel/func-def.h"
#include "toplevel/func-decl.h"

typedef enum ElHirTopLevelKind {
    EL_HIR_TOPLVL_FUNC_DEF,
    EL_HIR_TOPLVL_FUNC_DECL,
} ElHirTopLevelKind;

typedef struct ElHirTopLevelNode ElHirTopLevelNode;
struct ElHirTopLevelNode {
    ElHirTopLevelKind kind;
    union {
        ElHirFuncDef func_def;
        ElHirFuncDecl func_decl;
    } as;
    ElHirTopLevelNode* next;
};

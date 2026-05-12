#pragma once

#include "toplevel/func-def.h"

typedef enum ElHirTopLevelKind {
    EL_HIR_TOPLVL_FUNC_DEF,
} ElHirTopLevelKind;

typedef struct ElHirTopLevelNode ElHirTopLevelNode;
struct ElHirTopLevelNode {
    ElHirTopLevelKind kind;
    union {
        ElHirFuncDef func_def;
    } as;
    ElHirTopLevelNode* next;
};

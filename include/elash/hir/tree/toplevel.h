#pragma once

#include <elash/sema/type.h>
#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

#include "stmt/block.h"

typedef enum ElHirTopLevelKind {
    EL_HIR_TOPLVL_FUNC_DEF,
} ElHirTopLevelKind;

typedef struct ElHirFuncDefinition {
    ElSymbol* symbol;
    ElHirBlockStmtNode block;
} ElHirFuncDefinition;

typedef struct ElHirTopLevelNode ElHirTopLevelNode;
struct ElHirTopLevelNode {
    ElHirTopLevelKind kind;
    union {
        ElHirFuncDefinition func_def;
    } as;
    ElHirTopLevelNode* next;
};

ElHirTopLevelNode* el_hir_new_func_definition(ElDynArena* arena, ElSymbol* symbol, ElHirBlockStmtNode block);

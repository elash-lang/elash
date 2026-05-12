#include <elash/sema/type.h>
#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

#include <elash/hir/tree/stmt/block.h>

typedef struct ElHirTopLevelNode ElHirTopLevelNode;

typedef struct ElHirFuncDef {
    ElSymbol* symbol;
    ElHirBlockStmtNode block;
} ElHirFuncDef;

ElHirTopLevelNode* el_hir_new_func_def(ElDynArena* arena, ElSymbol* symbol, ElHirBlockStmtNode block);


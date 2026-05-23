#include <elash/sema/type.h>
#include <elash/sema/symbol.h>
#include <elash/util/dynarena.h>

#include <elash/hir/tree/stmt/block.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirFuncDef {
    ElSymbol* symbol;
    ElHirBlockStmt block;
} ElHirFuncDef;

ElHirDecl* el_hir_new_func_def(ElDynArena* arena, ElSymbol* symbol, ElHirBlockStmt block);

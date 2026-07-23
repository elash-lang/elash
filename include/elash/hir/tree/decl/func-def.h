#include <elash/hir/type.h>
#include <elash/hir/symbol.h>
#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/hir/tree/stmt/block.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirFuncDef {
    ElHirSymbol* symbol;
    ElHirBlockStmt block;
} ElHirFuncDef;

ElHirDecl* el_hir_new_func_def(ElDynArena* arena, ElSourceSpan span, ElHirSymbol* symbol, ElHirBlockStmt block);

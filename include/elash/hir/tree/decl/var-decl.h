#pragma once

#include <elash/hir/symbol.h>
#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirDecl ElHirDecl;

typedef struct ElHirVarDecl {
    ElHirSymbol* var;
} ElHirVarDecl;

ElHirDecl* el_hir_new_var_decl(ElDynArena* arena, ElSourceSpan span, ElHirSymbol* symbol);

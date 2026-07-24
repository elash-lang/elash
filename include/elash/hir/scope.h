#pragma once

#include <elash/defs/int-types.h>
#include <elash/defs/attr.h>
#include <elash/defs/sv.h>

#include <elash/hir/symbol.h>

typedef struct ElScopeEntry {
    ElStringView name;
    ElHirSymbol* symbol;
} ElScopeEntry;

typedef struct ElScope ElScope;
struct ElScope {
    ElScope* parent;

    ElScopeEntry* entries;
    usize capacity;
    usize count;
};

ElScope* el_hir_scope_new(ElScope* parent);
void el_hir_scope_free(ElScope* scope);

EL_ATTR_NODISCARD
bool el_hir_scope_insert(ElScope* scope, ElHirSymbol* symbol);

EL_ATTR_NODISCARD
bool el_hir_scope_insert_ex(ElScope* scope, ElStringView name, ElHirSymbol* symbol);

ElHirSymbol* el_hir_scope_lookup(ElScope* scope, ElStringView name);
ElHirSymbol* el_hir_scope_lookup_local(ElScope* scope, ElStringView name);
